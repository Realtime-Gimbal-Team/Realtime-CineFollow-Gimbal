#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>
#include <cmath>
#include <mutex>
#include <iomanip>
#include <algorithm> // For std::max and std::min

#include <ncnn/net.h>
#include <ncnn/mat.h>

#include "utils/SharedState.h"
#include "comm/UartDriver.h"
#include "vision/VisionNode.h"

std::atomic<bool> running(true);
SharedState state;

void signalHandler(int signum) {
    std::cout << "\n[System] Shutting down Iris Gimbal Node...\n";
    running = false;
}

// ==========================================
// Thread 1: Vision Perception (Greedy Matching Tracker with Fallback Recovery)
// ==========================================
void visionThread() {
    ncnn::Net yolov8_pose;
    if (yolov8_pose.load_param("../models/yolov8n-pose.param") || 
        yolov8_pose.load_model("../models/yolov8n-pose.bin")) {
        std::cerr << "[Vision] Error: Pose Model missing!" << std::endl;
        return;
    }

    // Immutable Baseline Polarities
    const float X_POLARITY = -1.0f;  
    const float Y_POLARITY = 1.0f; 
    
    const float TARGET_X = 160.0f;
    const float TARGET_Y = 100.0f; 

    VisionNode camera;
    if (!camera.init()) return;

    std::mutex frame_mutex;
    cv::Mat latest_frame;
    bool has_new_frame = false;

    // Asynchronous event-driven callback for frame capture
    camera.startCapture([&](const cv::Mat& frame) {
        if (!running) return;
        std::lock_guard<std::mutex> lock(frame_mutex);
        frame.copyTo(latest_frame);
        has_new_frame = true; 
    });

    std::cout << "[Vision] NAKED Mode Active. Tracking algorithm: Greedy Matching." << std::endl;

    static float last_target_x = TARGET_X;
    static float last_target_y = TARGET_Y;

    while(running) { 
        cv::Mat current_frame;
        {
            std::lock_guard<std::mutex> lock(frame_mutex);
            if (has_new_frame) {
                current_frame = latest_frame;
                has_new_frame = false;
            }
        }
        if (current_frame.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        const int input_size = 640; 
        ncnn::Mat in = ncnn::Mat::from_pixels_resize(current_frame.data, ncnn::Mat::PIXEL_BGR2RGB, 
                                                    current_frame.cols, current_frame.rows, 
                                                    input_size, input_size);
        const float norm_vals[3] = {1/255.f, 1/255.f, 1/255.f};
        in.substract_mean_normalize(0, norm_vals);

        ncnn::Extractor ex = yolov8_pose.create_extractor();
        ex.input("in0", in); 
        ncnn::Mat out;
        ex.extract("out0", out); 

        bool found = false;
        float min_distance = 9999.0f; 
        float best_x = TARGET_X;
        float best_y = TARGET_Y;

        // Fallback variables in case target moves completely out of bounds and returns
        float fallback_x = TARGET_X;
        float fallback_y = TARGET_Y;
        float max_fallback_conf = 0.0f;

        if (!out.empty() && out.h == 56) {
            for (int i = 0; i < out.w; i++) {
                float score = out.row(4)[i]; 
                if (score > 0.45f) {
                    float nose_x = out.row(5)[i] / 2.0f;
                    float nose_y = out.row(6)[i] / 2.0f;
                    float nose_conf = out.row(7)[i];

                    float current_x = nose_x;
                    float current_y = nose_y;

                    if (nose_conf <= 0.5f) {
                        float l_sh_x = out.row(20)[i]/2.0f, l_sh_y = out.row(21)[i]/2.0f;
                        float r_sh_x = out.row(23)[i]/2.0f, r_sh_y = out.row(24)[i]/2.0f;
                        current_x = (l_sh_x + r_sh_x) / 2.0f;
                        current_y = (l_sh_y + r_sh_y) / 2.0f - 20.0f; 
                    }
                    
                    // Track highest confidence target as a fallback safety mechanism
                    if (score > max_fallback_conf) {
                        max_fallback_conf = score;
                        fallback_x = current_x;
                        fallback_y = current_y;
                    }

                    float dx = current_x - last_target_x;
                    float dy = current_y - last_target_y;
                    float distance = std::sqrt(dx*dx + dy*dy);

                    if (distance < min_distance && distance < 150.0f) {
                        min_distance = distance;
                        best_x = current_x;
                        best_y = current_y;
                        found = true;
                    }
                }
            }
        }

        // 🌟 FATAL DEADLOCK FIX: If tracking is lost but someone is in frame, re-acquire highest confidence target
        if (!found && max_fallback_conf > 0.45f) {
            best_x = fallback_x;
            best_y = fallback_y;
            found = true;
        }

        if (found) {
            last_target_x = best_x;
            last_target_y = best_y;
            float raw_x = (best_x - TARGET_X) * X_POLARITY;
            float raw_y = (best_y - TARGET_Y) * Y_POLARITY;
            state.updateVision(raw_x, raw_y, true); 
        } else {
            state.updateVision(0.0f, 0.0f, false);
        }
    }
}

// ==========================================
// Thread 2: Control Planning (Symmetric Soft Ramp Version - FIX FOR YAW STUTTER)
// ==========================================
void controlThread() {
    const float KP_YAW = 0.005f;   
    const float KP_PITCH = 0.005f; 
    const float DEADZONE = 12.0f;  
    const float MAX_VEL = 0.4f;

    // 🌟 核心修正：废除非对称斜坡，回归绝对对称！
    // 统一使用 0.015 作为避震器的步长，双向平等过滤 YOLO 噪声。
    // 这将彻底砍掉高频锯齿波，消除 Yaw 轴的一顿一顿感。
    const float SYMMETRIC_STEP = 0.015f; 

    float cur_v_yaw = 0.0f;
    float cur_v_pitch = 0.0f;

    int log_counter = 0; 

    while (running) {
        auto start = std::chrono::steady_clock::now();
        VisionData vd = state.getVision();
        
        float tgt_v_x = 0.0f; 
        float tgt_v_y = 0.0f; 

        if (vd.target_found) {
            // Soft Deadzone Logic
            float active_err_x = 0.0f;
            if (vd.ex > DEADZONE) active_err_x = vd.ex - DEADZONE;
            else if (vd.ex < -DEADZONE) active_err_x = vd.ex + DEADZONE;

            float active_err_y = 0.0f;
            if (vd.ey > DEADZONE) active_err_y = vd.ey - DEADZONE;
            else if (vd.ey < -DEADZONE) active_err_y = vd.ey + DEADZONE;

            tgt_v_x = active_err_x * KP_YAW;
            tgt_v_y = active_err_y * KP_PITCH;

            tgt_v_x = std::clamp(tgt_v_x, -MAX_VEL, MAX_VEL);
            tgt_v_y = std::clamp(tgt_v_y, -MAX_VEL, MAX_VEL);
        }

        // 🌟 修正后的对称线性斜坡 (Symmetric Linear Ramp)
        // 无论加速还是减速，都以相同的 SYMMETRIC_STEP 平滑过渡
        if (cur_v_yaw < tgt_v_x) {
            cur_v_yaw = std::min(cur_v_yaw + SYMMETRIC_STEP, tgt_v_x);
        } else if (cur_v_yaw > tgt_v_x) {
            cur_v_yaw = std::max(cur_v_yaw - SYMMETRIC_STEP, tgt_v_x);
        }

        if (cur_v_pitch < tgt_v_y) {
            cur_v_pitch = std::min(cur_v_pitch + SYMMETRIC_STEP, tgt_v_y);
        } else if (cur_v_pitch > tgt_v_y) {
            cur_v_pitch = std::max(cur_v_pitch - SYMMETRIC_STEP, tgt_v_y);
        }

        // Neuro-aligned command dispatch
        state.updateCommand(cur_v_yaw, cur_v_pitch);

        std::this_thread::sleep_until(start + std::chrono::milliseconds(10));
    }
}

// ==========================================
// Thread 3: UART Dispatch
// ==========================================
void uartThread() {
    UartDriver uart;
    if (!uart.init("/dev/serial0")) return;

    while (running) {
        auto start = std::chrono::steady_clock::now();
        VelocityCommand cmd = state.getCommand();
        uart.sendVelocity(cmd.pitch_vel, cmd.yaw_vel);
        std::this_thread::sleep_until(start + std::chrono::milliseconds(20));
    }
}

int main() {
    std::signal(SIGINT, signalHandler);
    std::cout << "==================================================" << std::endl;
    std::cout << "[System] Iris Gimbal Active (Final Release v1.2.1)" << std::endl;
    std::cout << "==================================================" << std::endl;

    std::thread t1(visionThread);
    std::thread t2(controlThread);
    std::thread t3(uartThread);

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (t1.joinable()) t1.join();
    if (t2.joinable()) t2.join();
    if (t3.joinable()) t3.join();
    
    return 0;
}