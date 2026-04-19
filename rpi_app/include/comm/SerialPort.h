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
    // Open the serial port and configure the parameters
    bool openPort(const std::string& device, int baud_rate) {
        serial_fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
        if (serial_fd == -1) return false;

        struct termios options;
        tcgetattr(serial_fd, &options);

        // Set the baud rate to 115200 for initial debugging [39].
        speed_t speed = (baud_rate == 115200) ? B115200 : B921600;
        cfsetispeed(&options, speed);
        cfsetospeed(&options, speed);

        // Data format: 8 data bits, no parity, 1 stop bit (8N1) [40].
        options.c_cflag &= ~PARENB;
        options.c_cflag &= ~CSTOPB;
        options.c_cflag &= ~CSIZE;
        options.c_cflag |= CS8;

        tcsetattr(serial_fd, TCSANOW, &options);
        return true;
    }

    //Send arbitrary data structures (template function)
    template<typename T>
    void sendData(const T& data) {
        if (serial_fd != -1) {
            write(serial_fd, &data, sizeof(T));
        }
    }

    // Close the serial port
    void closePort() {
        if (serial_fd != -1) close(serial_fd);
    }
};
