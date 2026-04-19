#pragma once
#include <stdint.h>

// 强制取消编译器结构体对齐优化，防止不同编译器自动填充导致数据错位 [cite: 89, 94, 95]
#pragma pack(push, 1)

// 基础帧头定义 [cite: 96]
struct FrameHeader {
    uint8_t head1 = 0x55;      // 帧头 1 [cite: 44, 98]
    uint8_t head2 = 0xAA;      // 帧头 2 [cite: 44, 99]
    uint8_t cmd_id;            // 指令功能码 [cite: 44, 100]
    uint8_t data_len;          // Payload 的有效数据长度 [cite: 44, 101]
};

// CMD 0x01: 设定目标角度 Payload [cite: 48, 104]
struct PayloadSetAngle {
    float pitch;               // 俯仰角，单位：度 [cite: 53, 106]
    float yaw;                 // 偏航角，单位：度 [cite: 54, 107]
};

// 下发目标角度的完整数据帧 [cite: 123, 124]
struct FrameSetAngle {
    FrameHeader header;        // [cite: 125]
    PayloadSetAngle payload;   // [cite: 126]
    uint8_t checksum;          // 校验和 [cite: 44, 127]
    uint8_t tail = 0x0D;       // 帧尾：固定为 0x0D (\r) [cite: 44, 128]
};

// 恢复编译器默认对齐方式 [cite: 130]
#pragma pack(pop)