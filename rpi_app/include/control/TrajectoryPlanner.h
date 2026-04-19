#pragma once
#include <algorithm>
#include <cmath>

class TrajectoryPlanner {
private:
    float max_velocity; // Maximum angular velocity limit (rad/s)
    float max_accel;    // Maximum angular acceleration (rad/s²) — determines the smoothness of motion
    float deadzone;     // Pixel dead zone — suppresses jitter and micro-oscillations
    
    float current_v_pitch = 0.0f;
    float current_v_yaw = 0.0f;

public:
    TrajectoryPlanner(float v_max, float a_max, float dz) 
        : max_velocity(v_max), max_accel(a_max), deadzone(dz) {}

    // Compute smoothed velocity (dt is the control period, e.g., 100 Hz = 0.01 s)
    void computeVelocity(float ex, float ey, bool found, float dt, float& out_vp, float& out_vy) {
        if (!found) {
            // When the target is lost, apply smooth damped braking instead of an abrupt stop.
            smoothDecelerate(dt);
            out_vp = current_v_pitch;
            out_vy = current_v_yaw;
            return;
        }

        // Central deadzone filtering (Deadzone)
        if (std::abs(ex) < deadzone) ex = 0.0f;
        if (std::abs(ey) < deadzone) ey = 0.0f;

        // IBVS visual servoing: pixel error → target velocity mapping
        float target_v_yaw = ex * 0.015f;   // Empirical tuning parameters, can be externalized later
        float target_v_pitch = ey * 0.015f;

        target_v_yaw = std::clamp(target_v_yaw, -max_velocity, max_velocity);
        target_v_pitch = std::clamp(target_v_pitch, -max_velocity, max_velocity);

        // Core S-curve logic: limit the maximum rate of change of velocity (acceleration)
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
