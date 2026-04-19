#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>
#include <cmath>
#include <mutex>
#include <ncnn/net.h>
#include <ncnn/mat.h>

// 引入工程规范的四大核心组件 (请确保这些头文件在你的 include 目录下)
#include "utils/SharedState.h"
#include "control/TrajectoryPlanner.h"
#include "comm/UartDriver.h"
#include "vision/VisionNode.h"

// 全局运行标志位与共享状态中枢
std::atomic<bool> running(true);
SharedState state;

// 信号处理：捕捉 Ctrl+C，优雅退出，防止终端卡死
void signalHandler(int signum) {
    std::cout << "\n[System] Interrupt signal (" << signum << ") received. Shutting down...\n";
    running = false;
}

// ==========================================
// 线程 1：视觉感知线程 (Vision Thread)
// ==========================================
void visionThread() {
    // ==========================================
    // 1. NCNN AI 大脑初始化 (只执行一次)
    // ==========================================
    ncnn::Net yolov8;
    yolov8.opt.use_vulkan_compute = false; 
    yolov8.opt.num_threads = 4;            

    // 🚨 注意路径：如果你的可执行文件在 build 里，模型在工程根目录的 models 里，必须用 ../
    if (yolov8.load_param("../models/yolov8n.param") || yolov8.load_model("../models/yolov8n.bin")) {
        std::cerr << "[Vision] Error: NCNN load failed! Please check if the path is correct." << std::endl;
        return;
    }
    std::cout << "[Vision] NCNN YOLOv8 Engine Loaded." << std::endl;

    // ==========================================
    // 2. 硬件摄像头初始化
    // ==========================================
    VisionNode camera;
    if (!camera.init()) return;

    // 🌟 线程安全的帧缓冲区 🌟
    std::mutex frame_mutex;
    cv::Mat latest_frame;
    bool has_new_frame = false;

    // 生产者：极速拷贝硬件图像，绝对不阻塞
    camera.startCapture([&](const cv::Mat& frame) {
        if (!running) return;
        std::lock_guard<std::mutex> lock(frame_mutex);
        frame.copyTo(latest_frame);
        has_new_frame = true; 
    });

    std::cout << "[Vision] Pipeline Active. Real-time Inference Started." << std::endl;

    // ==========================================
    // 3. 消费者：AI 独立推理主循环
    // ==========================================
    while(running) { 
        cv::Mat current_frame;
        
        // 【抢图】安全地从缓冲区抢出最新的一帧图像
        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            if (has_new_frame) {
                latest_frame.copyTo(current_frame);
                has_new_frame = false;
            }
        }

        // 如果没拿到新图，休眠 2ms 让出 CPU
        if (current_frame.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            continue; 
        }

        // 【前向传播】图像缩放与推理
        const int target_size = 320; 
        ncnn::Mat in = ncnn::Mat::from_pixels_resize(
            current_frame.data, 
            ncnn::Mat::PIXEL_BGR2RGB, 
            current_frame.cols, current_frame.rows, 
            target_size, target_size
        );

        const float norm_vals[3] = {1 / 255.0f, 1 / 255.0f, 1 / 255.0f};
        in.substract_mean_normalize(0, norm_vals);

        ncnn::Extractor ex = yolov8.create_extractor();
        ex.input("in0", in); 
        ncnn::Mat out;
        ex.extract("out0", out); 

        // ==========================================
        // 🎯 核心后处理：YOLOv8 Bounding Box 解码
        // ==========================================
        bool target_found = false;
        float cx = 0.0f;
        float cy = 0.0f;

        // 获取输出数据指针 (YOLOv8 的输出通常是 [1, 84, 8400])
        float* data = out.row(0);
        float max_score = 0.0f;
        float best_cx = 0.0f;
        float best_cy = 0.0f;

        // 遍历 8400 个候选框
        for (int i = 0; i < 8400; i++) {
            // 索引 0-3 是框的坐标，从索引 4 开始是类别置信度
            // 我们只关心“人 (Person)”，即类别 0，它的置信度在 data[4 * 8400 + i]
            float score = data[4 * 8400 + i]; 

            // 过滤阈值：置信度大于 45%，且只追踪画面里最确定的那个人
            if (score > 0.45f && score > max_score) {
                max_score = score;
                best_cx = data[0 * 8400 + i]; // 获取基于 320 尺度的中心点 X
                best_cy = data[1 * 8400 + i]; // 获取基于 320 尺度的中心点 Y
                target_found = true;
            }
        }

        if (target_found) {
            // 将 320 尺度下的坐标，等比例放大回真实的 640x480 物理世界中
            cx = best_cx * (current_frame.cols / (float)target_size);
            cy = best_cy * (current_frame.rows / (float)target_size);

            // 计算物理像素误差
            float ex_err = cx - (current_frame.cols / 2.0f);
            float ey_err = cy - (current_frame.rows / 2.0f);
            
            // 下发追踪指令！
            state.updateVision(ex_err, ey_err, true); 
        } else {
            // 视野里没找到人，刹车
            state.updateVision(0.0f, 0.0f, false);
        }
    }
}

// ==========================================
// 线程 2：控制与轨迹规划线程 (Control Thread)
// ==========================================
void controlThread() {
    // 实例化 S 曲线规划器
    // 参数: 允许最大速度 2.0 rad/s, 最大加速度 5.0 rad/s^2, 像素死区 10px
    TrajectoryPlanner planner(0.5f, 0.5f, 10.0f);
    
    const int control_hz = 100; // 100Hz 高频控制计算
    const float dt = 1.0f / control_hz;

    std::cout << "[Control] Thread started. S-Curve Planner Active @ 100Hz." << std::endl;

    while (running) {
        auto start = std::chrono::steady_clock::now();

        // 1. 从共享内存读取最新视觉状态
        VisionData vd = state.getVision();
        
        // 2. 运行 S 曲线平滑计算 (将像素偏差转化为平滑角速度)
        float vp, vy;
        planner.computeVelocity(vd.ex, vd.ey, vd.target_found, dt, vp, vy);
        
        // 3. 将计算好的速度写入指令区
        state.updateCommand(vp, vy);

        // 严格定时
        std::this_thread::sleep_until(start + std::chrono::milliseconds(1000 / control_hz));
    }
}

// ==========================================
// 线程 3：串口通信下发线程 (UART TX Thread)
// ==========================================
void uartThread() {
    UartDriver uart;
    
    // 初始化串口：这里我们锁定了外部引脚 /dev/serial0
    if (!uart.init("/dev/serial0")) {
        std::cerr << "[UART] Error: Failed to open serial port!" << std::endl;
        return;
    }
    std::cout << "[UART] Port Opened. 14-byte Protocol Transmission Active @ 50Hz." << std::endl;

    const int uart_hz = 50; // 50Hz 通信下发频率

    while (running) {
        auto start = std::chrono::steady_clock::now();

        // 1. 读取控制线程算好的最新速度
        VelocityCommand cmd = state.getCommand();
        
        // 2. 封装 14 字节并下发
        uart.sendVelocity(cmd.pitch_vel, cmd.yaw_vel);

        // 终端打印当前下发的速度，方便你肉眼核对电机转向
        std::cout << "TX -> Pitch Vel: " << cmd.pitch_vel << " | Yaw Vel: " << cmd.yaw_vel << "\r" << std::flush;

        // 严格定时
        std::this_thread::sleep_until(start + std::chrono::milliseconds(1000 / uart_hz));
    }
    std::cout << std::endl; // 退出时换行
}


// ==========================================
// 主函数：系统初始化与守护
// ==========================================
int main() {
    // 绑定 Ctrl+C 信号
    std::signal(SIGINT, signalHandler);

    std::cout << "========================================" << std::endl;
    std::cout << "[System] Realtime CineFollow Gimbal RPi5" << std::endl;
    std::cout << "[System] IBVS Mode | Multi-threading ON " << std::endl;
    std::cout << "========================================" << std::endl;

    // 启动多线程并发
    std::thread t1(visionThread);
    std::thread t2(controlThread);
    std::thread t3(uartThread);

    // 主守护线程：只要 running 为 true，主线程就不退出
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // 等待所有子线程安全汇合
    std::cout << "\n[System] Joining threads..." << std::endl;
    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();
    if (t3.joinable()) t3.join();
    
    std::cout << "[System] Shutdown complete." << std::endl;
    return 0;
}