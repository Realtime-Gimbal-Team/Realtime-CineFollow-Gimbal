#pragma once
#include <iostream>
#include <functional>
#include <opencv2/opencv.hpp>

// Integrate the professor’s full-mark core library
#include "libcam2opencv.h" 
#include <libcamera/libcamera.h>

class VisionNode {
private:
    // libcamera: essential low-level environment manager
    std::unique_ptr<libcamera::CameraManager> cm;
    // Professor-encapsulated camera class
    Libcam2OpenCV libcam;
    bool is_initialized = false;

public:
    VisionNode() {}

    ~VisionNode() {
        stop();
    }

    // Initialize the camera environment
    bool init() {
        cm = std::make_unique<libcamera::CameraManager>();
        cm->start(); // Start low-level hardware scanning

        if (cm->cameras().empty()) {
            std::cerr << "[Vision Error] CameraManager found no cameras! Check ribbon cable." << std::endl;
            return false;
        }
        
        std::cout << "[Vision] libcamera Manager started. Found cameras." << std::endl;
        is_initialized = true;
        return true;
    }

    // Register callbacks and start the camera
    void startCapture(std::function<void(const cv::Mat&)> onFrameReady) {
        if (!is_initialized) {
            std::cerr << "[Vision Error] Call init() before startCapture()." << std::endl;
            return;
        }

        // 1. Register callbacks: bridge the Mat provided by the professor into our own processing core
        // Note: The professor’s callback includes a libcamera::ControlList; here we ignore it and only use the cv::Mat.
        libcam.registerCallback([onFrameReady](const cv::Mat &mat, const libcamera::ControlList &) {
            // Ensure the matrix is not empty before passing it upstream
            if (!mat.empty()) {
                onFrameReady(mat); 
            }
        });

        // 2. Configure parameters: use 640×480 to ensure frame rate and processing speed (within the capabilities of RPi 5)
        Libcam2OpenCVSettings settings;
        settings.width = 640;
        settings.height = 480;
        settings.framerate = 30; 
        // You can even adjust settings.brightness and settings.contrast here to adapt to your lab lighting conditions.

        // 3. Establish the hardware data stream
        libcam.start(*cm, settings);
        std::cout << "[Vision] Hardware Capture Stream Started! Resolution: 640x480" << std::endl;
    }

    // Safe shutdown
    void stop() {
        if (is_initialized) {
            libcam.stop();
            cm->stop();
            is_initialized = false;
        }
    }
};
