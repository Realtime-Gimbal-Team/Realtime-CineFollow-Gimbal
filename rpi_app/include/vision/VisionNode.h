#pragma once
#include <iostream>
#include <functional>
#include <opencv2/opencv.hpp>

// 引入教授的满分核心库
#include "libcam2opencv.h" 
#include <libcamera/libcamera.h>

class VisionNode {
private:
    // libcamera 必须的底层环境管理器
    std::unique_ptr<libcamera::CameraManager> cm;
    // 教授封装的相机类
    Libcam2OpenCV libcam;
    bool is_initialized = false;

public:
    VisionNode() {}

    ~VisionNode() {
        stop();
    }

    // 初始化相机环境
    bool init() {
        cm = std::make_unique<libcamera::CameraManager>();
        cm->start(); // 启动底层硬件扫描

        if (cm->cameras().empty()) {
            std::cerr << "[Vision Error] CameraManager found no cameras! Check ribbon cable." << std::endl;
            return false;
        }
        
        std::cout << "[Vision] libcamera Manager started. Found cameras." << std::endl;
        is_initialized = true;
        return true;
    }

    // 注册回调并启动相机
    void startCapture(std::function<void(const cv::Mat&)> onFrameReady) {
        if (!is_initialized) {
            std::cerr << "[Vision Error] Call init() before startCapture()." << std::endl;
            return;
        }

        // 1. 注册回调：把教授传出来的 mat 桥接到我们自己的神经中枢里
        // 注意：教授的回调带有 libcamera::ControlList，我们这里直接忽略它，只取 cv::Mat
        libcam.registerCallback([onFrameReady](const cv::Mat &mat, const libcamera::ControlList &) {
            // 确保矩阵不为空，再向上传递
            if (!mat.empty()) {
                onFrameReady(mat); 
            }
        });

        // 2. 配置参数：使用 640x480 以保证帧率和处理速度 (符合 RPi5 算力)
        Libcam2OpenCVSettings settings;
        settings.width = 640;
        settings.height = 480;
        settings.framerate = 30; 
        // 你甚至可以在这里调 settings.brightness, settings.contrast 适应你们实验室的光线

        // 3. 正式打通硬件流
        libcam.start(*cm, settings);
        std::cout << "[Vision] Hardware Capture Stream Started! Resolution: 640x480" << std::endl;
    }

    // 安全关闭
    void stop() {
        if (is_initialized) {
            libcam.stop();
            cm->stop();
            is_initialized = false;
        }
    }
};