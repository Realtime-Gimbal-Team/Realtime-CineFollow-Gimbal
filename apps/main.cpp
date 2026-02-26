#include <iostream>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>
#include <vector>
#include <iomanip>
#include "../include/vision/YoloDetector.h"

// 【底层硬件依赖：Linux POSIX 串口控制】
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// ==========================================================
// 【V1.2 协议数据结构：严格单字节对齐】
// 警告：不要增删任何字段，总长度必须为严丝合缝的 14 字节！
// ==========================================================
#pragma pack(push, 1) 
struct FrameHeader {
    uint8_t head1 = 0x55;    // 帧头 1
    uint8_t head2 = 0xAA;    // 帧头 2
    uint8_t cmd_id = 0x01;   // CMD 0x01: 设定目标角度
    uint8_t data_len = 0x08; // Payload 长度: 两个 float 共 8 字节
};

struct PayloadSetAngle {
    float pitch; // 根据 V1.1/V1.2 协议，必须先发 Pitch 
    float yaw;   // 再发 Yaw
};

struct FrameSetAngle {
    FrameHeader header;
    PayloadSetAngle payload;
    uint8_t checksum;
    uint8_t tail = 0x0D;     // 固定的帧尾
};
#pragma pack(pop)

int main() {
    std::cout << "[System] Booting Realtime-CineFollow-Gimbal (Protocol V1.2 RAW MODE)..." << std::endl;
    
    // ==========================================================
    // 1. 初始化物理串口 (/dev/serial0) - 终极防御版
    // ==========================================================
    // 批判性修正 1：移除 O_NDELAY，强制转为阻塞模式。
    // 必须确保 Linux 驱动把数据全部吞下后，write() 才能返回！
    int serial_fd = open("/dev/serial0", O_RDWR | O_NOCTTY);
    if (serial_fd == -1) {
        std::cerr << "[Fatal Error] Failed to open /dev/serial0! Check config.txt and wiring." << std::endl;
        return -1;
    }
    
    struct termios options;
    tcgetattr(serial_fd, &options);

    // 批判性修正 2：强制开启 Raw Mode (原始二进制模式)
    // 绝对禁止 Linux 内核将 0x0D(回车), 0x0A(换行), 0x03(Ctrl+C) 等特殊字节进行转义或吞咽！
    cfmakeraw(&options); 

    cfsetispeed(&options, B115200); 
    cfsetospeed(&options, B115200);
    
    // 纯净 8N1 配置，无校验，1 停止位
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB; 
    options.c_cflag &= ~CSTOPB; 
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;     
    
    // 致命修正 3：必须显式关闭硬件流控 (RTS/CTS)！
    // 否则在没有连接硬件握手线的情况下，数据会被永久憋在内核缓冲区发不出去。
    options.c_cflag &= ~CRTSCTS; 
    
    tcsetattr(serial_fd, TCSANOW, &options);
    
    // 开火前，清空底层管道里可能积压的历史垃圾数据
    tcflush(serial_fd, TCIOFLUSH); 
    
    std::cout << "[System] Hardware UART Armed in RAW MODE. Link is UP." << std::endl;

    // ==========================================================
    // 2. 初始化视觉大脑与 ISP
    // ==========================================================
    YoloDetector detector;
    if (!detector.loadModel("../models/model.ncnn.param", "../models/model.ncnn.bin")) {
        std::cerr << "[Fatal Error] YOLO Model load failed!" << std::endl;
        return -1;
    }

    cv::VideoCapture cap(0, cv::CAP_V4L2);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    if (!cap.isOpened()) {
        std::cerr << "[Fatal Error] Camera failed!" << std::endl; 
        return -1;
    }

    cv::Mat frame;
    std::cout << "[System] Warming up Camera..." << std::endl;
    for(int i = 0; i < 30; i++) cap >> frame; 

    std::cout << "==================================================" << std::endl;
    std::cout << "[System] Entering Cinematic Tracking Loop (20Hz High-Speed Mode)." << std::endl;
    std::cout << "==================================================" << std::endl;

    // --- 实时追踪死循环 ---
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "[Error] Frame dropped!" << std::endl;
            break;
        }

        std::vector<Object> objects = detector.detect(frame);

        float target_yaw = 0.0f;
        float target_pitch = 0.0f;

        if (!objects.empty()) {
            const Object& target = objects[0];
            
            float cx = target.rect.x + target.rect.width / 2.0f;
            float cy = target.rect.y + target.rect.height * 0.333f;

            // 图像坐标系到云台坐标系的极简映射
            float err_x = cx - 320.0f;
            float err_y = 240.0f - cy; 

            // 极简 P 控制比例映射
            target_yaw = err_x * 0.05f;
            target_pitch = err_y * 0.05f;

            // ==========================================================
            // 3. 组装 V1.2 协议数据包
            // ==========================================================
            FrameSetAngle cmd_frame;
            cmd_frame.payload.pitch = target_pitch;
            cmd_frame.payload.yaw = target_yaw;

            // 【V1.2 终极校验和】：CMD_ID(1) + Data_Len(1) + Payload(8)
            uint8_t sum = 0;
            sum += cmd_frame.header.cmd_id;
            sum += cmd_frame.header.data_len;
            
            uint8_t* payload_ptr = (uint8_t*)&cmd_frame.payload;
            for(int i = 0; i < cmd_frame.header.data_len; i++) {
                sum += payload_ptr[i];
            }
            cmd_frame.checksum = sum; // 自动截断为 uint8_t

            // ==========================================================
            // 4. 跨物理层发射与严苛监控
            // ==========================================================
            int bytes_written = write(serial_fd, &cmd_frame, sizeof(FrameSetAngle));

            if (bytes_written < 0) {
                std::cerr << "\n[Fatal Error] TX Failed! Linux driver swallowed the data." << std::endl;
            } else if (bytes_written != sizeof(FrameSetAngle)) {
                std::cerr << "\n[Warning] Partial Write! Only sent " << bytes_written << " bytes." << std::endl;
            } else {
                // 强制将内核缓冲区的数据推入物理引脚，防止在 Linux 内存中排队积压
                tcdrain(serial_fd); 

                // 终端监视器打印 (十六进制显示 Checksum 以便对账)
                std::cout << "[V1.2 TX SUCCESS] Pitch: " << std::fixed << std::setprecision(2) << target_pitch 
                          << "°, Yaw: " << target_yaw 
                          << "° | Checksum: 0x" << std::hex << std::uppercase << (int)sum << std::dec << std::endl;
            }

        } else {
            std::cout << "[Searching] No target. Holding position." << std::endl;
        }

        // ==========================================================
        // 【系统心跳：20Hz】
        // 维持 50ms 频率，测试 Pico 端的解包抗压能力。
        // ==========================================================
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    close(serial_fd);
    return 0;
}