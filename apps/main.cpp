#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono> // 引入 C++11 高精度时间库

int main() {
    std::cout << "[System] Initializing Vision Module..." << std::endl;
    
    cv::VideoCapture cap(0, cv::CAP_V4L2);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    if (!cap.isOpened()) {
        std::cerr << "Error: Cannot open camera!" << std::endl;
        return -1;
    }

    cv::Mat frame;
    std::cout << "[System] Warming up sensor (waiting for Auto-Exposure)..." << std::endl;
    // 强制预热 60 帧，保证画面不黑
    for(int i = 0; i < 30; i++) {
        cap >> frame;
    }

    if (frame.empty()) {
        std::cerr << "Error: Blank frame grabbed" << std::endl;
        return -1;
    }

    // --- 【核心漏洞修复：注入微秒级时间戳】 ---
    // 1. 记录画面刚刚存入 frame 矩阵的瞬间绝对时间
    auto capture_time = std::chrono::high_resolution_clock::now();
    // 2. 将时间点转换为微秒 (microseconds) 级别的长整型数值
    auto time_micro = std::chrono::time_point_cast<std::chrono::microseconds>(capture_time).time_since_epoch().count();
    
    std::cout << "[Latency Profiler] Frame captured at absolute time: " << time_micro << " us" << std::endl;

    // 为了直观验证，我们将时间戳直接画在测试图片上
    std::string time_str = "Capture Time: " + std::to_string(time_micro) + " us";
    cv::putText(frame, time_str, cv::Point(20, 50), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
    cv::putText(frame, "Latency Foundation Ready!", cv::Point(20, 90), 
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);

    // 保存图片
    bool isSaved = cv::imwrite("timestamp_test.jpg", frame);
    
    if (isSaved) {
        std::cout << "[Success] Frame saved to timestamp_test.jpg!" << std::endl;
    } else {
        std::cerr << "[Error] Failed to save image." << std::endl;
    }

    cap.release();
    return 0;
}