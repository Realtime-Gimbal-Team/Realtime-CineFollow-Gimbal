#pragma once
#include <algorithm>
#include <cmath>

class TrajectoryPlanner {
private:
    float max_velocity; // 最大角速度界限 (rad/s)
    float max_accel;    // 最大角加速度 (rad/s^2) - 决定运镜的丝滑程度
    float deadzone;     // 像素死区 - 抑制呼吸颤抖
    
    float current_v_pitch = 0.0f;
    float current_v_yaw = 0.0f;

public:
    TrajectoryPlanner(float v_max, float a_max, float dz) 
        : max_velocity(v_max), max_accel(a_max), deadzone(dz) {}

    // 计算平滑速度 (dt 为控制周期，例如 100Hz = 0.01s)
    void computeVelocity(float ex, float ey, bool found, float dt, float& out_vp, float& out_vy) {
        if (!found) {
            // 目标丢失，执行丝滑阻尼刹车，而不是瞬间急停
            smoothDecelerate(dt);
            out_vp = current_v_pitch;
            out_vy = current_v_yaw;
            return;
        }

        // 中心死区过滤 (Deadzone)
        if (std::abs(ex) < deadzone) ex = 0.0f;
        if (std::abs(ey) < deadzone) ey = 0.0f;

        // IBVS 视觉伺服：像素偏差 -> 目标速度映射
        float target_v_yaw = ex * 0.015f;   // 调参经验值，后续可外置
        float target_v_pitch = ey * 0.015f;

        target_v_yaw = std::clamp(target_v_yaw, -max_velocity, max_velocity);
        target_v_pitch = std::clamp(target_v_pitch, -max_velocity, max_velocity);

        // 核心 S 曲线逻辑：限制最大速度变化率 (加速度)
        float max_dv = max_accel * dt; 

        current_v_yaw += std::clamp(target_v_yaw - current_v_yaw, -max_dv, max_dv);
        current_v_pitch += std::clamp(target_v_pitch - current_v_pitch, -max_dv, max_dv);

        out_vp = current_v_pitch;
        out_vy = current_v_yaw;
    }

private:
    void smoothDecelerate(float dt) {
        float max_dv = max_accel * dt;
        current_v_yaw += std::clamp(0.0f - current_v_yaw, -max_dv, max_dv);
        current_v_pitch += std::clamp(0.0f - current_v_pitch, -max_dv, max_dv);
    }
};