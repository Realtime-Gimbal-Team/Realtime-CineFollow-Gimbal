#include "../../include/vision/YoloDetector.h"
#include <iostream>

YoloDetector::YoloDetector() {
    // 强行锁死输入分辨率为 320，榨干树莓派性能
    target_size = 320; 
    
    ncnn::Option opt;
    opt.num_threads = 4; // 火力全开 4 核心
    yolo.opt = opt;
}

YoloDetector::~YoloDetector() {
    yolo.clear();
}

bool YoloDetector::loadModel(const char* param_path, const char* bin_path) {
    int ret_param = yolo.load_param(param_path);
    int ret_bin = yolo.load_model(bin_path);
    if (ret_param != 0 || ret_bin != 0) {
        std::cerr << "[YOLO] Failed to load model! Check file paths." << std::endl;
        return false;
    }
    std::cout << "[YOLO] NCNN Model loaded successfully!" << std::endl;
    return true;
}

std::vector<Object> YoloDetector::detect(const cv::Mat& bgr_image) {
    std::vector<Object> objects;
    if (bgr_image.empty()) return objects;

    int img_w = bgr_image.cols;
    int img_h = bgr_image.rows;
    ncnn::Mat in = ncnn::Mat::from_pixels_resize(bgr_image.data, ncnn::Mat::PIXEL_BGR2RGB, 
                                                 img_w, img_h, target_size, target_size);

    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    in.substract_mean_normalize(0, norm_vals);

    ncnn::Extractor ex = yolo.create_extractor();
    // 官方导出的 NCNN 模型，出入口名字绝对是 in0 和 out0
    ex.input("in0", in); 

    ncnn::Mat out;
    ex.extract("out0", out); 

    // --- 纯净版 84 维张量解析 ---
    if (!out.empty()) {
        std::vector<int> class_ids;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;

        int num_boxes = 0;
        int num_features = 0;
        cv::Mat out_mat;

        if (out.dims == 3 || out.dims == 2) {
            num_features = out.h; 
            num_boxes = out.w;    
            out_mat = cv::Mat(num_boxes, num_features, CV_32F);
            for (int f = 0; f < num_features; f++) {
                const float* ptr = out.dims == 3 ? out.channel(0).row(f) : out.row(f);
                for (int b = 0; b < num_boxes; b++) {
                    out_mat.at<float>(b, f) = ptr[b];
                }
            }
        }

        // 强行打印内存形状诊断。这一次，必须是 Features: 84！
        std::cout << "[Debug] Parsed Tensor -> Boxes: " << num_boxes << ", Features: " << num_features << std::endl;

        if (num_boxes == 0 || num_features != 84) {
            std::cerr << "[Error] Tensor shape mismatch! Features must be 84." << std::endl;
            return objects;
        }

        float scale_x = (float)img_w / target_size;
        float scale_y = (float)img_h / target_size;

        for (int i = 0; i < num_boxes; i++) {
            float max_class_score = 0.0f;
            int class_id = -1;
            
            // 第 4 到 83 位是 80 个类别的概率分布
            for (int c = 4; c < num_features; c++) {
                float score = out_mat.at<float>(i, c);
                if (score > max_class_score) {
                    max_class_score = score;
                    class_id = c - 4; 
                }
            }

            // 我们只要“人”(class_id == 0) 并且置信度大于 40%
            if (class_id == 0 && max_class_score > 0.4f) {
                // 前 4 位是 cx, cy, w, h
                float cx = out_mat.at<float>(i, 0);
                float cy = out_mat.at<float>(i, 1);
                float bw = out_mat.at<float>(i, 2);
                float bh = out_mat.at<float>(i, 3);

                int left   = (int)((cx - 0.5f * bw) * scale_x);
                int top    = (int)((cy - 0.5f * bh) * scale_y);
                int width  = (int)(bw * scale_x);
                int height = (int)(bh * scale_y);

                class_ids.push_back(class_id);
                confidences.push_back(max_class_score);
                boxes.push_back(cv::Rect(left, top, width, height));
            }
        }

        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, 0.4f, 0.45f, indices);

        for (int idx : indices) {
            Object obj;
            obj.rect = boxes[idx];
            obj.label = class_ids[idx];
            obj.prob = confidences[idx];
            objects.push_back(obj);
        }
    }

    return objects;
}