#pragma once
#include <stdint.h>

#pragma pack(push, 1) 

// --- Receive: Raspberry Pi -> Pico ---
struct FrameHeader {
    uint8_t head1 = 0x55;
    uint8_t head2 = 0xAA;
    uint8_t cmd_id = 0x01;
    uint8_t data_len = 0x08; // Two floats, totaling 8 bytes
};

// 🌟 Modified: the semantics have been changed from angle to velocity
struct PayloadSetVelocity {
    float pitch_vel; 
    float yaw_vel;   
};

struct FrameSetVelocity {
    FrameHeader header;
    PayloadSetVelocity payload;
    uint8_t checksum;
    uint8_t tail = 0x0D;
};

// --- Send: Pico -> Raspberry Pi (status feedback) ---
struct FrameHeaderTx {
    uint8_t head1 = 0xAA;
    uint8_t head2 = 0x55; 
    uint8_t cmd_id = 0x02;
    uint8_t data_len = 0x08; 
};

struct PayloadFeedback {
    float pitch_current_vel; // Since there is no encoder, feeding back velocity is more meaningful than feeding back a fake angle
    float yaw_current_vel;
};

struct FrameFeedback {
    FrameHeaderTx header;
    PayloadFeedback payload;
    uint8_t checksum;
    uint8_t tail = 0x0D;
};

#pragma pack(pop)
