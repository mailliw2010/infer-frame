/**
 * @file yolov8_onnx_test.cc
 * @brief 使用真实 YOLOv8 ONNX 模型测试后端推理
 */

#include "inference/backend_factory.h"
#include "inference/base/types.h"
#include "utils/one_logger.hpp"
#include <opencv2/opencv.hpp>
#include <chrono>

using namespace infer_frame;
using namespace infer_frame::backend;
using namespace infer_frame::base;

/**
 * @brief 预处理图像为 YOLOv8 输入格式
 * @param image 输入图像
 * @param input_tensor 输出 Tensor (1, 3, 640, 640)
 */
void preprocessImage(const cv::Mat& image, Tensor* input_tensor) {
    // YOLOv8 输入：[1, 3, 640, 640], RGB, 归一化到 [0, 1]
    const int input_h = 640;
    const int input_w = 640;
    
    // Resize 和填充
    cv::Mat resized;
    cv::resize(image, resized, cv::Size(input_w, input_h));
    
    // BGR -> RGB
    cv::Mat rgb;
    cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);
    
    // 归一化并转换为 CHW 格式
    rgb.convertTo(rgb, CV_32F, 1.0 / 255.0);
    
    // 将数据拷贝到 Tensor (使用模板方法 getPtr<float>())
    float* data_ptr = input_tensor->getPtr<float>();
    int channels = 3;
    int img_area = input_h * input_w;
    
    // OpenCV Mat 是 HWC 格式，需要转为 CHW
    for (int c = 0; c < channels; c++) {
        for (int h = 0; h < input_h; h++) {
            for (int w = 0; w < input_w; w++) {
                int dst_idx = c * img_area + h * input_w + w;
                data_ptr[dst_idx] = rgb.at<cv::Vec3f>(h, w)[c];
            }
        }
    }
}

/**
 * @brief 后处理 YOLOv8 输出
 * @param output_tensor 输出 Tensor
 */
void postprocessOutput(Tensor* output_tensor) {
    // YOLOv8 输出通常是 [1, 84, 8400] 或类似格式
    // 84 = 4(bbox) + 80(classes)
    auto desc = output_tensor->getDesc();
    auto& shape = desc.shape_;
    
    LOG_INFO("Output tensor shape: [{}, {}, {}]", 
             shape.size() > 0 ? shape[0] : 0,
             shape.size() > 1 ? shape[1] : 0, 
             shape.size() > 2 ? shape[2] : 0);
    
    float* data = output_tensor->getPtr<float>();
    
    // 简单统计：找最大值
    int total_elements = 1;
    for (auto dim : shape) {
        total_elements *= dim;
    }
    
    if (total_elements > 0 && data != nullptr) {
        float max_val = data[0];
        float min_val = data[0];
        double sum = 0.0;
        
        for (int i = 0; i < std::min(total_elements, 1000); i++) {
            max_val = std::max(max_val, data[i]);
            min_val = std::min(min_val, data[i]);
            sum += data[i];
        }
        
        LOG_INFO("Output statistics (first 1000 elements):");
        LOG_INFO("  Min: {:.6f}", min_val);
        LOG_INFO("  Max: {:.6f}", max_val);
        LOG_INFO("  Mean: {:.6f}", sum / std::min(total_elements, 1000));
    }
}

int main() {
    LOG_INFO("======================================");
    LOG_INFO("  YOLOv8 TensorRT Inference Test");
    LOG_INFO("======================================");
    
    // 初始化后端注册
    initializeBackends();
    
    // 获取工厂实例
    auto& factory = BackendFactory::getInstance();
    
    // 配置 TensorRT 后端（使用 ONNX 模型，TensorRT 会自动转换）
    BackendConfig config;
    config.backend_type = BackendType::kTensorRT;
    config.model_path = "/home/mic-711/xcd/infer-frame/algorithm/yolov8/model/yolov8s_quant.onnx";
    config.device_id = 0;
    
    LOG_INFO("Loading YOLOv8 ONNX model with TensorRT: {}", config.model_path);
    
    // 创建后端
    auto backend = factory.createBackend(config);
    if (!backend) {
        LOG_ERROR("Failed to create TensorRT backend");
        return -1;
    }
    
    LOG_INFO("✓ TensorRT backend created and model loaded successfully");
    
    // 获取模型输入输出信息
    auto input_infos = backend->getInputInfos();
    auto output_infos = backend->getOutputInfos();
    
    LOG_INFO("\nModel Information:");
    LOG_INFO("  Inputs: {}", input_infos.size());
    for (size_t i = 0; i < input_infos.size(); i++) {
        const auto& info = input_infos[i];
        LOG_INFO("    [{}] {}: shape=[{},{},{},{}]", 
                 i, info.name, 
                 info.shape.size() > 0 ? info.shape[0] : 0,
                 info.shape.size() > 1 ? info.shape[1] : 0,
                 info.shape.size() > 2 ? info.shape[2] : 0,
                 info.shape.size() > 3 ? info.shape[3] : 0);
    }
    
    LOG_INFO("  Outputs: {}", output_infos.size());
    for (size_t i = 0; i < output_infos.size(); i++) {
        const auto& info = output_infos[i];
        LOG_INFO("    [{}] {}: shape=[{},{},{}]", 
                 i, info.name,
                 info.shape.size() > 0 ? info.shape[0] : 0,
                 info.shape.size() > 1 ? info.shape[1] : 0,
                 info.shape.size() > 2 ? info.shape[2] : 0);
    }
    
    // 加载真实测试图像
    LOG_INFO("\n--- Loading Test Image ---");
    std::string image_path = "/home/mic-711/xcd/infer-frame/algorithm/yolov8/data/pic.png";
    cv::Mat test_image = cv::imread(image_path);
    if (test_image.empty()) {
        LOG_ERROR("Failed to load test image: {}", image_path);
        return -1;
    }
    LOG_INFO("Loaded test image: {}x{} from {}", 
             test_image.cols, test_image.rows, image_path);
    
    // 创建输入 Tensor
    TensorDesc input_desc;
    input_desc.shape_ = {1, 3, 640, 640};
    input_desc.data_type_ = nndeploy::base::dataTypeOf<float>();
    
    // 创建设备（CPU）
    auto device = nndeploy::device::getDefaultHostDevice();
    
    // 创建并分配 Tensor 内存
    auto input_tensor = new Tensor(device, input_desc, "input");
    LOG_INFO("Created input tensor: shape=[1,3,640,640]");
    
    // 预处理
    preprocessImage(test_image, input_tensor);
    LOG_INFO("✓ Image preprocessed");
    
    // 创建输出 Tensor 占位符
    std::vector<Tensor*> inputs = {input_tensor};
    std::vector<Tensor*> outputs;  // 后端会自动分配输出
    
    // 执行推理
    LOG_INFO("\n--- Running Inference ---");
    auto start = std::chrono::high_resolution_clock::now();
    
    auto status = backend->infer(inputs, outputs);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (status.ok()) {
        LOG_INFO("✓ Inference succeeded!");
        LOG_INFO("  Inference time: {} ms", duration.count());
        LOG_INFO("  Output tensors: {}", outputs.size());
        
        // 后处理输出
        if (!outputs.empty()) {
            LOG_INFO("\n--- Processing Output ---");
            postprocessOutput(outputs[0]);
        }
    } else {
        LOG_ERROR("✗ Inference failed: {}", status.message());
    }
    
    // 清理资源
    delete input_tensor;
    for (auto* tensor : outputs) {
        delete tensor;
    }
    
    // 反初始化后端
    backend->deinit();
    
    LOG_INFO("\n======================================");
    LOG_INFO("  Test Completed!");
    LOG_INFO("======================================");
    
    return status.ok() ? 0 : -1;
}
