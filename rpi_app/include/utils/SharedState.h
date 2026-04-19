#pragma once
#include <mutex>

//Visual perception data
struct VisionData {
    float ex = 0.0f; // Pixel error (X)
    float ey = 0.0f; // Pixel error (Y)
    bool target_found = false;
};

// Velocity command transmission (based on IBVS open-loop control)
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
    void updateVision(float ex, float ey, bool found) {
        std::lock_guard<std::mutex> lock(vision_mtx);
        current_vision = {ex, ey, found};
    }

    //--- For the Control Thread to read ---
    VisionData getVision() {
        std::lock_guard<std::mutex> lock(vision_mtx);
        return current_vision;
    }

    //--- For the Control Thread to write ---
    void updateCommand(float p_vel, float y_vel) {
        std::lock_guard<std::mutex> lock(cmd_mtx);
        current_cmd = {p_vel, y_vel};
    }

    // --- For the UART Thread to read ---
    VelocityCommand getCommand() {
        std::lock_guard<std::mutex> lock(cmd_mtx);
        return current_cmd;
    }
};
