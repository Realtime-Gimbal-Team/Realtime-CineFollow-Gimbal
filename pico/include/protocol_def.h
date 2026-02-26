#pragma once
#include <stdint.h>

#pragma pack(push, 1) 
struct FrameHeader {
    uint8_t head1 = 0x55;
    uint8_t head2 = 0xAA;
    uint8_t cmd_id = 0x01;
    uint8_t data_len = 0x08;
};

struct PayloadSetAngle {
    float pitch; 
    float yaw;   
};

struct FrameSetAngle {
    FrameHeader header;
    PayloadSetAngle payload;
    uint8_t checksum;
    uint8_t tail = 0x0D;
};
#pragma pack(pop)