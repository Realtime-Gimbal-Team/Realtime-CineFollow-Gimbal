#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>
#include <cmath>
#include <mutex>
#include <iomanip>
#include <algorithm> // 用于 std::max 和 std::min

#include <ncnn/net.h>
#include <ncnn/mat.h>

#include "utils/SharedState.h"
// #include "control/TrajectoryPlanner.h" // 🚨 彻底注销，坚决不用！
#include "comm/UartDriver.h"
#include "vision/VisionNode.h"

std::atomic<bool> running(true);
SharedState state;

void signalHandler(int signum) {
    std::cout << "\n[System] Shutting down NAKED RPi5 Gimbal Node...\n";
    running = false;
}

// ==========================================
// 线程 1：视觉感知 (🚨 彻底剥离滤波，暴露出原始误差)
// ==========================================
void visionThread() {
    ncnn::Net yolov8_pose;
    if (yolov8_pose.load_param("../models/yolov8n-pose.param") || 
        yolov8_pose.load_model("../models/yolov8n-pose.bin")) {
        std::cerr << "[Vision] Error: Pose Model missing!" << std::endl;
        return;
    }

    // 保持你之前验证过的极性
    const float X_POLARITY = -1.0f;  
    const float Y_POLARITY = 1.0f; 
    
    const float TARGET_X = 160.0f;
    const float TARGET_Y = 140.0f; 

    VisionNode camera;
    if (!camera.init()) return;

    std::mutex frame_mutex;
    cv::Mat latest_frame;
    bool has_new_frame = false;

    camera.startCapture([&](const cv::Mat& frame) {
        if (!running) return;
        std::lock_guard<std::mutex> lock(frame_mutex);
        frame.copyTo(latest_frame);
        has_new_frame = true; 
    });

    std::cout << "[Vision] NAKED Mode Active. ALL FILTERS DISABLED." << std::endl;

    while(running) { 
        cv::Mat current_frame;
        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            if (has_new_frame) {
                current_frame = latest_frame;
                has_new_frame = false;
            }
        }
        if (current_frame.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        const int input_size = 640; 
        ncnn::Mat in = ncnn::Mat::from_pixels_resize(current_frame.data, ncnn::Mat::PIXEL_BGR2RGB, 
                                                    current_frame.cols, current_frame.rows, 
                                                    input_size, input_size);
        const float norm_vals[3] = {1/255.f, 1/255.f, 1/255.f};
        in.substract_mean_normalize(0, norm_vals);

        ncnn::Extractor ex = yolov8_pose.create_extractor();
        ex.input("in0", in); 
        ncnn::Mat out;
        ex.extract("out0", out); 

        bool found = false;
        float anchor_x = 160.0f, anchor_y = 160.0f;
        float max_s = 0.0f;

        if (!out.empty() && out.h == 56) {
            for (int i = 0; i < out.w; i++) {
                float score = out.row(4)[i]; 
                if (score > 0.45f && score > max_s) {
                    max_s = score;
                    float nose_x = out.row(5)[i] / 2.0f;
                    float nose_y = out.row(6)[i] / 2.0f;
                    float nose_conf = out.row(7)[i];

                    if (nose_conf > 0.5f) {
                        anchor_x = nose_x;
                        anchor_y = nose_y;
                    } else {
                        float l_sh_x = out.row(20)[i]/2.0f, l_sh_y = out.row(21)[i]/2.0f;
                        float r_sh_x = out.row(23)[i]/2.0f, r_sh_y = out.row(24)[i]/2.0f;
                        anchor_x = (l_sh_x + r_sh_x) / 2.0f;
                        anchor_y = (l_sh_y + r_sh_y) / 2.0f - 20.0f; 
                    }
                    found = true;
                }
            }
        }

        if (found) {
            // 🚨 算出最纯粹的像素误差，没有任何平滑、没有任何衰减
            float raw_x = (anchor_x - TARGET_X) * X_POLARITY;
            float raw_y = (anchor_y - TARGET_Y) * Y_POLARITY;

            // 直接将最原始的误差塞给控制线程
            state.updateVision(raw_x, raw_y, true); 
        } else {
            state.updateVision(0.0f, 0.0f, false);
        }
    }
}

// ==========================================
// 线程 2：控制规划 (🚨 物理轴对齐版 - 修正参数 1 为 Yaw 的错误)
// ==========================================
void controlThread() {
    // 采用极简 P 控制进行物理验证
    const float KP_YAW = 0.003f;   
    const float KP_PITCH = 0.003f; 
    const float DEADZONE = 12.0f;  

    while (running) {
        auto start = std::chrono::steady_clock::now();
        VisionData vd = state.getVision();
        
        float v_x = 0.0f; // 水平速度 (欲发给 Yaw)
        float v_y = 0.0f; // 垂直速度 (欲发给 Pitch)

        if (vd.target_found) {
            if (std::abs(vd.ex) > DEADZONE) v_x = vd.ex * KP_YAW;
            if (std::abs(vd.ey) > DEADZONE) v_y = vd.ey * KP_PITCH;

            // 限速
            v_x = std::max(-0.6f, std::min(0.6f, v_x));
            v_y = std::max(-0.6f, std::min(0.6f, v_y));
        }

        // 🚨 拨乱反正核心：
        // 既然你的硬件把第一个参数当成了 Yaw，第二个当成了 Pitch
        // 那么：updateCommand(水平速度 v_x, 垂直速度 v_y)
        // 这样软件的 v_x 就会准确流向硬件的水平电机！
        state.updateCommand(v_x, v_y);

        std::this_thread::sleep_until(start + std::chrono::milliseconds(10));
    }
}

// ==========================================
// 线程 3：串口下发
// ==========================================
void uartThread() {
    UartDriver uart;
    if (!uart.init("/dev/serial0")) return;

    while (running) {
        auto start = std::chrono::steady_clock::now();
        VelocityCommand cmd = state.getCommand();
        uart.sendVelocity(cmd.pitch_vel, cmd.yaw_vel);
        std::this_thread::sleep_until(start + std::chrono::milliseconds(20));
    }
}

int main() {
    std::signal(SIGINT, signalHandler);
    std::cout << "==================================================" << std::endl;
    std::cout << "[System] CineFollow DIAGNOSTIC NAKED BUILD Active" << std::endl;
    std::cout << "==================================================" << std::endl;

    std::thread t1(visionThread);
    std::thread t2(controlThread);
    std::thread t3(uartThread);

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();
    if (t3.joinable()) t3.join();
    
    return 0;
}