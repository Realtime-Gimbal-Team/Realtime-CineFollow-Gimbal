#pragma once
#include <string>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>

class SerialPort {
private:
    int serial_fd = -1;

public:
    // 打开串口并配置参数
    bool openPort(const std::string& device, int baud_rate) {
        serial_fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (serial_fd == -1) return false;

        struct termios options;
        tcgetattr(serial_fd, &options);

        // 设置波特率，初期调试使用 115200 [cite: 39]
        speed_t speed = (baud_rate == 115200) ? B115200 : B921600;
        cfsetispeed(&options, speed);
        cfsetospeed(&options, speed);

        // 数据格式：8 数据位，无校验，1 停止位 (8N1) [cite: 40]
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;

        tcsetattr(serial_fd, TCSANOW, &options);
        return true;
    }

    // 发送任意数据结构（模板函数）
    template<typename T>
    void sendData(const T& data) {
        if (serial_fd != -1) {
            write(serial_fd, &data, sizeof(T));
        }
    }

    // 关闭串口
    void closePort() {
        if (serial_fd != -1) close(serial_fd);
    }
};