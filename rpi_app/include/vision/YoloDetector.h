#pragma once
#include <opencv2/opencv.hpp>
#include <ncnn/net.h> // Integrate the Tencent NCNN core library
#include <vector>

// Define a structure to store detected targets
struct Object {
    cv::Rect_<float> rect; // Bounding box of the target (x, y, width, height)
    int label;             // Class label (e.g., 0 represents a person)
    float prob;            // Confidence (0.0–1.0)
};

class YoloDetector {
public:
    YoloDetector();
    ~YoloDetector();

    bool loadModel(const char* param_path, const char* bin_path);


    std::vector<Object> detect(const cv::Mat& bgr_image);

private:
    ncnn::Net yolo;
    int target_size; // Input size for NCNN inference
};
