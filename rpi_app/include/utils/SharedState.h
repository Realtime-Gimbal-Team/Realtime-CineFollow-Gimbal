#pragma once
#include <mutex>

// 视觉感知数据
struct VisionData {
    float ex = 0.0f; // 像素误差 X
    float ey = 0.0f; // 像素误差 Y
    bool target_found = false;
};

// 速度下发指令 (基于 IBVS 开环控制)
struct VelocityCommand {
    float pitch_vel = 0.0f; // rad/s
    float yaw_vel = 0.0f;   // rad/s
};

class SharedState {
private:
    VisionData current_vision;
    VelocityCommand current_cmd;
    std::mutex vision_mtx;
    std::mutex cmd_mtx;

public:
    // --- 供 Vision Thread 写入 ---
    void updateVision(float ex, float ey, bool found) {
        std::lock_guard<std::mutex> lock(vision_mtx);
        current_vision = {ex, ey, found};
    }

    // --- 供 Control Thread 读取 ---
    VisionData getVision() {
        std::lock_guard<std::mutex> lock(vision_mtx);
        return current_vision;
    }

    // --- 供 Control Thread 写入 ---
    void updateCommand(float p_vel, float y_vel) {
        std::lock_guard<std::mutex> lock(cmd_mtx);
        current_cmd = {p_vel, y_vel};
    }

    // --- 供 UART Thread 读取 ---
    VelocityCommand getCommand() {
        std::lock_guard<std::mutex> lock(cmd_mtx);
        return current_cmd;
    }
};