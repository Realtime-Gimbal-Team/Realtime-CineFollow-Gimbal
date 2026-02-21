#pragma once
#include <stdint.h>

// 强制编译器不要优化内存对齐，保证数据挨个紧凑存放
#pragma pack(push, 1)

// 0x01 指令的模具：接收目标角度
struct PayloadSetAngle {
    float pitch;
    float yaw;
};

// 0x10 指令的模具：上报当前状态
struct PayloadTelemetry {
    float current_pitch;
    float current_yaw;
    float battery_voltage;
    uint8_t status_flags;
};

#pragma pack(pop)