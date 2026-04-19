#pragma once
#include <opencv2/opencv.hpp>
#include <ncnn/net.h> // 引入腾讯 NCNN 核心库
#include <vector>

// 定义一个结构体，用来存放识别到的目标
struct Object {
    cv::Rect_<float> rect; // 目标的边界框 (x, y, 宽, 高)
    int label;             // 类别标签 (比如 0 代表人)
    float prob;            // 置信度 (0.0 ~ 1.0)
};

class YoloDetector {
public:
    YoloDetector();
    ~YoloDetector();

    // 核心接口 1：加载你在上一步下载的模型文件
    bool loadModel(const char* param_path, const char* bin_path);

    // 核心接口 2：传入一帧图像，返回画面中所有的目标
    std::vector<Object> detect(const cv::Mat& bgr_image);

private:
    ncnn::Net yolo;
    int target_size; // NCNN 推理时的输入尺寸
};