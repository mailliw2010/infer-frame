/**
 * @file yolov8_onnx_test_nndeploy.cc
 * @brief 使用 NNDeploy 原生 API 测试 YOLOv8 TensorRT 推理
 */

#include <opencv2/opencv.hpp>
#include <memory>
#include <iostream>
#include "nndeploy/base/glic_stl_include.h"
#include "nndeploy/base/time_profiler.h"
#include "nndeploy/device/device.h"
#include "nndeploy/device/tensor.h"
#include "nndeploy/inference/inference.h"
#include "nndeploy/inference/inference_param.h"

using namespace nndeploy;

// 简单的图像预处理
cv::Mat preprocessImage(const cv::Mat& image, int target_w = 640, int target_h = 640) {
    cv::Mat resized;
    cv::resize(image, resized, cv::Size(target_w, target_h));
    
    cv::Mat rgb;
    cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);
    
    cv::Mat normalized;
    rgb.convertTo(normalized, CV_32FC3, 1.0 / 255.0);
    
    return normalized;
}

int main() {
    std::cout << "[INFO] YOLOv8 TensorRT Inference Test (NNDeploy Native API)" << std::endl;
    std::cout << "[INFO] Loading YOLOv8 ONNX model with TensorRT" << std::endl;

    // 模型路径
    std::string model_path = "/home/mic-711/xcd/infer-frame/algorithm/yolov8/model/yolov8s_quant.onnx";
    
    // 创建 TensorRT 推理参数
    base::InferenceType inference_type = base::kInferenceTypeTensorRt;
    auto infer_param = std::make_shared<inference::InferenceParam>();
    infer_param->is_path_ = true;
    infer_param->model_value_.push_back(model_path);
    
    // 设置设备类型（GPU）
    base::DeviceType device_type;
    device_type.code_ = base::kDeviceTypeCodeCuda;
    device_type.device_id_ = 0;
    infer_param->device_type_ = device_type;
    
    // TensorRT 特定参数
    infer_param->precision_type_ = base::kPrecisionTypeFp16;  // 使用 FP16
    
    // 创建推理实例
    auto inference_instance = inference::createInference(inference_type);
    if (!inference_instance) {
        std::cerr << "[ERROR] Failed to create TensorRT inference instance" << std::endl;
        return -1;
    }
    
    // 设置参数
    inference_instance->setParam(infer_param.get());
    
    // 初始化
    auto status = inference_instance->init();
    if (status != base::kStatusCodeOk) {
        std::cerr << "[ERROR] Failed to initialize TensorRT: " << status.desc() << std::endl;
        return -1;
    }
    std::cout << "[INFO] ✓ TensorRT inference initialized successfully" << std::endl;
    
    // 加载测试图片
    std::string test_image_path = "/home/mic-711/xcd/infer-frame/algorithm/yolov8/data/pic.png";
    cv::Mat image = cv::imread(test_image_path);
    if (image.empty()) {
        std::cerr << "[ERROR] Failed to load image: " << test_image_path << std::endl;
        return -1;
    }
    std::cout << "[INFO] Loaded test image: " << image.cols << "x" << image.rows 
              << " from " << test_image_path << std::endl;
    
    // 预处理图像
    cv::Mat preprocessed = preprocessImage(image, 640, 640);
    
    // 转换为 CHW 格式
    std::vector<cv::Mat> channels;
    cv::split(preprocessed, channels);
    
    // 获取输入 tensor
    auto input_tensor_map = inference_instance->getAllInputTensorMap();
    if (input_tensor_map.empty()) {
        std::cerr << "[ERROR] No input tensors found!" << std::endl;
        return -1;
    }
    
    // 获取第一个输入tensor
    auto input_tensor = input_tensor_map.begin()->second;
    std::cout << "[INFO] Input tensor name: " << input_tensor_map.begin()->first << std::endl;
    
    // 打印 tensor 形状
    auto shape = input_tensor->getShape();
    std::cout << "[INFO] Input tensor shape: [";
    for (size_t i = 0; i < shape.size(); ++i) {
        std::cout << shape[i];
        if (i < shape.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    // 复制数据到 tensor
    float* tensor_data = (float*)input_tensor->getPtr();
    if (!tensor_data) {
        std::cerr << "[ERROR] Failed to get tensor data pointer!" << std::endl;
        return -1;
    }
    
    for (int c = 0; c < 3; ++c) {
        memcpy(tensor_data + c * 640 * 640, channels[c].data, 640 * 640 * sizeof(float));
    }
    
    std::cout << "[INFO] ✓ Image preprocessed and copied to input tensor" << std::endl;
    
    // 执行推理
    auto start = std::chrono::high_resolution_clock::now();
    status = inference_instance->run();
    auto end = std::chrono::high_resolution_clock::now();
    
    if (status != base::kStatusCodeOk) {
        std::cerr << "[ERROR] Inference failed: " << status.desc() << std::endl;
        return -1;
    }
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // 获取输出
    auto output_tensor_map = inference_instance->getAllOutputTensorMap();
    std::cout << "[INFO] ✓ Inference succeeded! "
              << "Inference time: " << duration.count() << " ms, "
              << "Output tensors: " << output_tensor_map.size() << std::endl;
    
    // 打印输出信息
    for (auto& kv : output_tensor_map) {
        auto output_shape = kv.second->getShape();
        std::cout << "[INFO] Output '" << kv.first << "' shape: [";
        for (size_t i = 0; i < output_shape.size(); ++i) {
            std::cout << output_shape[i];
            if (i < output_shape.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
        
        // 打印前几个输出值
        float* output_data = (float*)kv.second->getPtr();
        if (output_data) {
            std::cout << "[INFO] First 10 values: ";
            int total_elements = 1;
            for (auto dim : output_shape) total_elements *= dim;
            int num_to_print = std::min(10, total_elements);
            for (int i = 0; i < num_to_print; ++i) {
                std::cout << output_data[i] << " ";
            }
            std::cout << std::endl;
        }
    }
    
    // 清理
    inference_instance->deinit();
    std::cout << "[INFO] TensorRT inference deinitialized" << std::endl;
    std::cout << "[INFO] Test Completed!" << std::endl;
    
    return 0;
}
