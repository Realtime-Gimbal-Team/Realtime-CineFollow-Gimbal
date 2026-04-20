#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>
#include <cmath>
#include <mutex>
#include <iomanip>
#include <algorithm>

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
// Thread 1: Vision Perception (Final Interactive Version with Gesture Switch)
// ==========================================
void visionThread() {
    ncnn::Net yolov8_pose;
    if (yolov8_pose.load_param("../models/yolov8n-pose.param") || 
        yolov8_pose.load_model("../models/yolov8n-pose.bin")) {
        std::cerr << "[Vision] Error: Pose Model missing!" << std::endl;
        return;
    }

    const float X_POLARITY = -1.0f;  
    const float Y_POLARITY = 1.0f; 
    const float TARGET_X = 160.0f;
    const float TARGET_Y = 100.0f; 

    VisionNode camera;
    if (!camera.init()) return;

    std::mutex frame_mutex;
    cv::Mat latest_frame;
    bool has_new_frame = false;

    camera.startCapture([&](const cv::Mat& frame) {
        if (!running) return;
        std::lock_guard<std::mutex> lock(frame_mutex);
        frame.copyTo(latest_frame);
        has_new_frame = true; 
    });

    std::cout << "[Vision] NAKED Mode Active. Tracking: Greedy. Gesture Switch: ON." << std::endl;

    static float last_target_x = TARGET_X;
    static float last_target_y = TARGET_Y;

    // 🌟 Global variables for gesture interaction state machine
    static bool is_tracking = true;         // Main switch for gimbal tracking
    static int gesture_hold_frames = 0;     // Counter for continuous hand-raising frames
    static int cooldown_frames = 0;         // Cooldown lock counter

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
        float best_x = TARGET_X, best_y = TARGET_Y;
        
        // 🌟 Record the hands state of the best target
        float best_lwrist_y = 0.0f, best_lwrist_conf = 0.0f;
        float best_rwrist_y = 0.0f, best_rwrist_conf = 0.0f;

        // Fallback variables
        float fallback_x = TARGET_X, fallback_y = TARGET_Y;
        float fb_lwrist_y = 0.0f, fb_lwrist_conf = 0.0f;
        float fb_rwrist_y = 0.0f, fb_rwrist_conf = 0.0f;
        float max_fallback_conf = 0.0f;

        if (!out.empty() && out.h == 56) {
            for (int i = 0; i < out.w; i++) {
                float score = out.row(4)[i]; 
                if (score > 0.45f) {
                    float nose_x = out.row(5)[i] / 2.0f;
                    float nose_y = out.row(6)[i] / 2.0f;
                    float nose_conf = out.row(7)[i];

                    // Extract wrists (Point 9=LWrist, Point 10=RWrist)
                    float current_lwrist_y = out.row(33)[i] / 2.0f;
                    float current_lwrist_conf = out.row(34)[i];
                    float current_rwrist_y = out.row(36)[i] / 2.0f;
                    float current_rwrist_conf = out.row(37)[i];

                    float current_x = nose_x;
                    float current_y = nose_y;

                    if (nose_conf <= 0.5f) {
                        float l_sh_x = out.row(20)[i]/2.0f, l_sh_y = out.row(21)[i]/2.0f;
                        float r_sh_x = out.row(23)[i]/2.0f, r_sh_y = out.row(24)[i]/2.0f;
                        current_x = (l_sh_x + r_sh_x) / 2.0f;
                        current_y = (l_sh_y + r_sh_y) / 2.0f - 20.0f; 
                    }
                    
                    if (score > max_fallback_conf) {
                        max_fallback_conf = score;
                        fallback_x = current_x; fallback_y = current_y;
                        fb_lwrist_y = current_lwrist_y; fb_lwrist_conf = current_lwrist_conf;
                        fb_rwrist_y = current_rwrist_y; fb_rwrist_conf = current_rwrist_conf;
                    }

                    float dx = current_x - last_target_x;
                    float dy = current_y - last_target_y;
                    float distance = std::sqrt(dx*dx + dy*dy);

                    if (distance < min_distance && distance < 150.0f) {
                        min_distance = distance;
                        best_x = current_x; best_y = current_y;
                        best_lwrist_y = current_lwrist_y; best_lwrist_conf = current_lwrist_conf;
                        best_rwrist_y = current_rwrist_y; best_rwrist_conf = current_rwrist_conf;
                        found = true;
                    }
                }
            }
        }

        if (!found && max_fallback_conf > 0.45f) {
            best_x = fallback_x; best_y = fallback_y;
            best_lwrist_y = fb_lwrist_y; best_lwrist_conf = fb_lwrist_conf;
            best_rwrist_y = fb_rwrist_y; best_rwrist_conf = fb_rwrist_conf;
            found = true;
        }

        // 🌟 Core: Update cooldown lock
        if (cooldown_frames > 0) cooldown_frames--;

        if (found) {
            // 🌟 Core: Gesture interaction judgment logic
            bool is_raising_hand = false;
            // Condition: Left or right wrist confidence is high enough, and wrist Y coordinate is less than nose (above head/nose)
            if ((best_lwrist_conf > 0.5f && best_lwrist_y < best_y) ||
                (best_rwrist_conf > 0.5f && best_rwrist_y < best_y)) {
                is_raising_hand = true;
            }

            if (is_raising_hand && cooldown_frames == 0) {
                gesture_hold_frames++;
                // At 86ms latency, 17 frames is approximately 1.5 seconds
                if (gesture_hold_frames >= 17) { 
                    is_tracking = !is_tracking;     // Toggle state!
                    gesture_hold_frames = 0;
                    cooldown_frames = 35;           // Lock for about 3 seconds, prevent immediate switching

                    std::cout << "\n======================================" << std::endl;
                    if (is_tracking) {
                        std::cout << ">>> [GESTURE] TRACKING RESUMED <<<" << std::endl;
                    } else {
                        std::cout << ">>> [GESTURE] TRACKING PAUSED <<<" << std::endl;
                    }
                    std::cout << "======================================\n" << std::endl;
                } else {
                    // Print progress bar for debugging reference
                    std::cout << "[Gesture] Holding: " << gesture_hold_frames << "/17    \r" << std::flush;
                }
            } else {
                gesture_hold_frames = 0; // Reset immediately when hand is lowered to prevent false triggers
            }

            last_target_x = best_x;
            last_target_y = best_y;

            // 🌟 Core Gate: Send data if in Tracking state, otherwise send 0 to force gimbal to stop
            if (is_tracking) {
                float raw_x = (best_x - TARGET_X) * X_POLARITY;
                float raw_y = (best_y - TARGET_Y) * Y_POLARITY;
                state.updateVision(raw_x, raw_y, true); 
            } else {
                state.updateVision(0.0f, 0.0f, false);
            }

        } else {
            gesture_hold_frames = 0;
            state.updateVision(0.0f, 0.0f, false);
        }
    }
}

// ==========================================
// Thread 2: Control Planning (Retain your tuned parameters)
// ==========================================
void controlThread() {
    const float KP_YAW = 0.005f;   
    const float KP_PITCH = 0.005f; 
    const float DEADZONE = 12.0f;  
    const float MAX_VEL = 0.9f;
    const float SYMMETRIC_STEP = 0.015f; 

    float cur_v_yaw = 0.0f;
    float cur_v_pitch = 0.0f;

    while (running) {
        auto start = std::chrono::steady_clock::now();
        VisionData vd = state.getVision();
        
        float tgt_v_x = 0.0f; 
        float tgt_v_y = 0.0f; 

        if (vd.target_found) {
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

        if (cur_v_yaw < tgt_v_x) cur_v_yaw = std::min(cur_v_yaw + SYMMETRIC_STEP, tgt_v_x);
        else if (cur_v_yaw > tgt_v_x) cur_v_yaw = std::max(cur_v_yaw - SYMMETRIC_STEP, tgt_v_x);

        if (cur_v_pitch < tgt_v_y) cur_v_pitch = std::min(cur_v_pitch + SYMMETRIC_STEP, tgt_v_y);
        else if (cur_v_pitch > tgt_v_y) cur_v_pitch = std::max(cur_v_pitch - SYMMETRIC_STEP, tgt_v_y);

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
    std::cout << "[System] Iris Gimbal Active (Gesture Engine v1.0)" << std::endl;
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