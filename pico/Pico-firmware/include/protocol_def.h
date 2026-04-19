#pragma once
#include <stdint.h>

#pragma pack(push, 1) 

// --- 接收：树莓派 -> Pico ---
struct FrameHeader {
    uint8_t head1 = 0x55;
    uint8_t head2 = 0xAA;
    uint8_t cmd_id = 0x01;
    uint8_t data_len = 0x08; // 两个 float，共 8 字节
};

// 🌟 修改：语义从角度转为速度
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

// --- 发送：Pico -> 树莓派 (状态反馈) ---
struct FrameHeaderTx {
    uint8_t head1 = 0xAA;
    uint8_t head2 = 0x55; 
    uint8_t cmd_id = 0x02;
    uint8_t data_len = 0x08; 
};

struct PayloadFeedback {
    float pitch_current_vel; // 既然没编码器，反馈速度比反馈虚假的角度更有意义
    float yaw_current_vel;
};

struct FrameFeedback {
    FrameHeaderTx header;
    PayloadFeedback payload;
    uint8_t checksum;
    uint8_t tail = 0x0D;
};

#pragma pack(pop)