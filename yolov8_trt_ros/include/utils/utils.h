/*
 * @Author: ruoxi521
 * @Date: 2023-05-21 19:25:01
 * @LastEditors: ruoxi521
 * @LastEditTime: 2024-04-15 21:15:11
 * @FilePath: /yolov8_ros/yolov8_trt_ros/include/utils/utils.h
 * @Description: 
 * 
 * Copyright (c) 2023 by suruoxi521@gmail.com, All Rights Reserved. 
 */
#pragma once
#include <algorithm> 
#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <chrono>
#include <cmath>
#include <numeric> // std::iota 

using  namespace cv;

#define CHECK(status) \
    do\
    {\
        auto ret = (status);\
        if (ret != 0)\
        {\
            std::cerr << "Cuda failure: " << ret << std::endl;\
            abort();\
        }\
    } while (0)
struct alignas(float) Detection {
    //center_x center_y w h
    float bbox[4];
    float conf;  // bbox_conf * cls_conf
    int class_id;
};
static inline cv::Mat preprocess_img(cv::Mat& img, int input_w, int input_h, std::vector<int>& padsize) {
    int w, h, x, y;
    float r_w = input_w / (img.cols*1.0);
    float r_h = input_h / (img.rows*1.0);

    // 根据输入的宽高比例与图像的宽高比例来调整图像的大小和位置，以保持图像在输入区域内居中显示且不变形。
    if (r_h > r_w) {        
        w = input_w;
        h = r_w * img.rows;
        x = 0;
        y = (input_h - h) / 2;
    }
    else {
        w = r_h * img.cols;
        h = input_h;
        x = (input_w - w) / 2;
        y = 0;
    }
    cv::Mat re(h, w, CV_8UC3);
    cv::resize(img, re, re.size(), 0, 0, cv::INTER_LINEAR);
    cv::Mat out(input_h, input_w, CV_8UC3, cv::Scalar(128, 128, 128));
    re.copyTo(out(cv::Rect(x, y, re.cols, re.rows)));
    padsize.push_back(h);
    padsize.push_back(w);
    padsize.push_back(y);
    padsize.push_back(x);// int newh = padsize[0], neww = padsize[1], padh = padsize[2], padw = padsize[3];

    return out;
}
// cv::Rect get_rect(cv::Mat& img, float bbox[4], int INPUT_W, int INPUT_H) {
// 	int l, r, t, b;
// 	float r_w = INPUT_W / (img.cols * 1.0);
// 	float r_h = INPUT_H / (img.rows * 1.0);
// 	if (r_h > r_w) {
// 		l = bbox[0];
// 		r = bbox[2];
// 		t = bbox[1]- (INPUT_H - r_w * img.rows) / 2;
// 		b = bbox[3] - (INPUT_H - r_w * img.rows) / 2;
// 		l = l / r_w;
// 		r = r / r_w;
// 		t = t / r_w;
// 		b = b / r_w;
// 	}
// 	else {
// 		l = bbox[0] - bbox[2] / 2.f - (INPUT_W - r_h * img.cols) / 2;
// 		r = bbox[0] + bbox[2] / 2.f - (INPUT_W - r_h * img.cols) / 2;
// 		t = bbox[1] - bbox[3] / 2.f;
// 		b = bbox[1] + bbox[3] / 2.f;
// 		l = l / r_h;
// 		r = r / r_h;
// 		t = t / r_h;
// 		b = b / r_h;

// 	}
// 	return cv::Rect(l, t, r - l, b - t);
// }
// void slice(std::vector<float> a, int id1, int id2, std::vector<float>& b)
// {
// 	for (int i =0; i < id2-id1; i++) {
// 		b.push_back(a[i + id1]);
// 	}
// }

static std::vector<std::string> loadLabelsFromFile(std::string filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return {};  // Return empty vector on error
    }

    std::vector<std::string> labels;
    std::string line;

    while (std::getline(file, line)) {
        labels.push_back(line);
    }

    file.close();

    return labels;
}