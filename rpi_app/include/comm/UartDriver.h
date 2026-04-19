#pragma once
#include <iostream>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// Strictly enforce 1-byte alignment according to the Pico-side protocol.
#pragma pack(push, 1)
struct ControlPacket {
    uint8_t header1 = 0x55;
    uint8_t header2 = 0xAA;
    uint8_t cmd_id  = 0x01;
    uint8_t data_len = 0x08;
    float pitch_vel;
    float yaw_vel;
    uint8_t checksum;
    uint8_t tail = 0x0D;
};
#pragma pack(pop)

class UartDriver {
private:
    int serial_fd = -1;

    // A robust checksum algorithm: fully compliant with the Pico requirements
    void fill_checksum(ControlPacket& pkt) {
        uint8_t sum = pkt.cmd_id + pkt.data_len;
        uint8_t* payload_ptr = reinterpret_cast<uint8_t*>(&pkt.pitch_vel);
        for(int i = 0; i < 8; ++i) { 
            sum += payload_ptr[i];
        }
        pkt.checksum = sum;
    }

public:
    ~UartDriver() {
        if (serial_fd != -1) close(serial_fd);
    }

    bool init(const char* port = "/dev/serial0") {
        // Blocking mode (Blocking I/O)
        serial_fd = open(port, O_RDWR | O_NOCTTY);
        if (serial_fd == -1) return false;

        struct termios options;
        tcgetattr(serial_fd, &options);
        cfmakeraw(&options); // Pure binary stream to prevent interception by the Linux kernel
        cfsetispeed(&options, B115200);
        cfsetospeed(&options, B115200);
        
        options.c_cflag &= ~(PARENB | CSTOPB | CSIZE | CRTSCTS); 
        options.c_cflag |= (CLOCAL | CREAD | CS8);
        
        options.c_cc[VMIN]  = 1;
        options.c_cc[VTIME] = 0; 
        
        tcsetattr(serial_fd, TCSANOW, &options);
        tcflush(serial_fd, TCIOFLUSH);
        return true;
    }

    void sendVelocity(float p_vel, float y_vel) {
        if (serial_fd == -1) return;

        ControlPacket pkt;
        pkt.pitch_vel = p_vel;
        pkt.yaw_vel = y_vel;
        
        fill_checksum(pkt);

        int bytes_written = write(serial_fd, &pkt, sizeof(ControlPacket));
        if (bytes_written == sizeof(ControlPacket)) {
            tcdrain(serial_fd); // Force output to the physical pins
        } else {
            std::cerr << "[UART Warning] TX Incomplete or Linux Buffer Error." << std::endl;
        }
    }
};
