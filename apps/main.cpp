#include <iostream>
#include <thread>
#include <chrono>
// 引入刚刚写好的两个头文件
#include "../include/comm/protocol_def.h"
#include "../include/comm/SerialPort.h"

int main() {
    SerialPort serial;
    // 树莓派 5 默认的主硬件串口
    std::string port_name = "/dev/serial0"; 

    std::cout << "[System] Initializing UART on " << port_name << "..." << std::endl;
    // 以 115200 波特率打开串口 [cite: 39]
    if (!serial.openPort(port_name, 115200)) {
        std::cerr << "Failed to open port. Did you run with sudo?" << std::endl;
        return -1;
    }

    // 1. 组装测试数据包：设定目标角度 [cite: 48]
    FrameSetAngle frame;
    frame.header.cmd_id = 0x01; // 0x01 代表设定目标角度 [cite: 48]
    frame.header.data_len = sizeof(PayloadSetAngle);
    frame.payload.pitch = 0.0f;
    frame.payload.yaw = 15.5f;

    // 2. 计算强化的校验和 (CMD_ID + Data_Len + Payload 累加和的低 8 位) 
    uint8_t sum = 0;
    sum += frame.header.cmd_id;
    sum += frame.header.data_len;
    uint8_t* p = (uint8_t*)&frame.payload;
    for(size_t i = 0; i < sizeof(PayloadSetAngle); i++) {
        sum += p[i];
    }
    frame.checksum = sum; // uint8_t 自动保留低 8 位 [cite: 44]

    // 3. 循环发送测试
    std::cout << "[Test] Ready to send. Yaw=" << frame.payload.yaw << ", Checksum=" << (int)frame.checksum << std::endl;
    for(int i = 0; i < 5; i++) {
        serial.sendData(frame);
        std::cout << "Packet " << i + 1 << " sent!" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    serial.closePort();
    return 0;
}