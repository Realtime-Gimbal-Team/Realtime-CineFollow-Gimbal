#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#include "../include/vision/YoloDetector.h"

int main() {
    std::cout << "[System] Booting Realtime-CineFollow-Gimbal SITL Mode..." << std::endl;
    
    // 1. 初始化视觉大脑 (YOLO)
    YoloDetector detector;
    if (!detector.loadModel("../models/model.ncnn.param", "../models/model.ncnn.bin")) {
        return -1;
    }

    // 2. 初始化光学神经 (Camera)
    cv::VideoCapture cap(0, cv::CAP_V4L2);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    if (!cap.isOpened()) {
        std::cerr << "[Error] Camera failed!" << std::endl; return -1;
    }

    cv::Mat frame;
    std::cout << "[System] Warming up ISP..." << std::endl;
    for(int i = 0; i < 30; i++) cap >> frame; 

    std::cout << "==================================================" << std::endl;
    std::cout << "[System] Entering Cinematic Tracking Loop." << std::endl;
    std::cout << "[System] Press Ctrl+C in terminal to stop." << std::endl;
    std::cout << "==================================================" << std::endl;

    // --- 实时追踪死循环 (SITL 测试) ---
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "[Error] Frame dropped!" << std::endl;
            break;
        }

        auto start_time = std::chrono::high_resolution_clock::now();
        std::vector<Object> objects = detector.detect(frame);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto latency_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

        float target_yaw = 0.0f;
        float target_pitch = 0.0f;

        if (!objects.empty()) {
            // 默认取画面中置信度最高的第一个目标
            const Object& target = objects[0];
            
            // 【核心修正 1：物理几何中心 (X轴控制偏航)】
            float cx = target.rect.x + target.rect.width / 2.0f;
            
            // 【核心修正 2：摄影级黄金锚点 (Y轴控制俯仰，死锁面部/肩颈区域)】
            float cy = target.rect.y + target.rect.height * 0.333f;

            // 【核心修正 3：计算像素误差 (光轴绝对中心为 320, 240)】
            float err_x = cx - 320.0f;
            float err_y = 240.0f - cy; 

            // 【核心修正 4：极简 P-Controller (比例映射，防剧烈抖动)】
            // 假设 1 pixel 对应 0.05 度，这取决于相机的物理焦距，之后可以作为 PID 的参数微调
            target_yaw = err_x * 0.05f;
            target_pitch = err_y * 0.05f;

            std::cout << "[Locked] Latency: " << latency_ms << "ms | "
                      << "ErrX: " << err_x << ", ErrY: " << err_y << " | "
                      << "UART -> Yaw: " << target_yaw << "°, Pitch: " << target_pitch << "°" << std::endl;
        } else {
            std::cout << "[Searching] No target. Holding position. (Yaw: 0°, Pitch: 0°)" << std::endl;
        }

        // 稍微延时，模拟云台系统真实的控制刷新周期 (约 20Hz)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return 0;
}