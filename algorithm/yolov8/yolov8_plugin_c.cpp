/**
 * @file yolov8_plugin_c.cpp
 * @brief YOLOv8 C 风格插件实现（独立编译）
 * 
 * 特点：
 * 1. 实现 C 接口，避免 ABI 问题
 * 2. 独立 CMakeLists.txt，可单独编译
 * 3. 内部集成 TensorRT/ONNX，Backend 由用户指定
 * 4. 不依赖主程序的 BackendFactory
 */

#include "../../src/plugin/algo_plugin_interface.h"

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <memory>

// ============================================================================
// YOLOv8 算法实现类（C++ 内部实现）
// ============================================================================

class YOLOv8Impl {
 public:
  YOLOv8Impl() : initialized_(false) {}
  ~YOLOv8Impl() {
    deinit();
  }
  
  AlgoStatus init(const AlgoInitParam* param) {
    if (initialized_) {
      return ALGO_STATUS_ERROR_ALREADY_INITIALIZED;
    }
    
    if (!param || !param->model_path) {
      return ALGO_STATUS_ERROR_INVALID_PARAM;
    }
    
    model_path_ = param->model_path;
    backend_ = param->backend;
    device_id_ = param->device_id;
    
    // 解析 JSON 配置
    if (param->config_json) {
      // TODO: 解析 JSON 参数（conf_threshold, nms_threshold 等）
      conf_threshold_ = 0.25f;
      nms_threshold_ = 0.45f;
      input_width_ = 640;
      input_height_ = 640;
    }
    
    // 根据 Backend 类型初始化推理引擎
    switch (backend_) {
      case ALGO_BACKEND_TENSORRT:
        std::cout << "[YOLOv8] Initializing TensorRT backend..." << std::endl;
        // TODO: 初始化 TensorRT
        // backend_impl_ = std::make_unique<TensorRTBackend>();
        break;
      case ALGO_BACKEND_ONNXRUNTIME:
        std::cout << "[YOLOv8] Initializing ONNX Runtime backend..." << std::endl;
        // TODO: 初始化 ONNX Runtime
        // backend_impl_ = std::make_unique<ONNXRuntimeBackend>();
        break;
      default:
        std::cout << "[YOLOv8] Backend not supported: " << backend_ << std::endl;
        return ALGO_STATUS_ERROR_BACKEND_NOT_SUPPORTED;
    }
    
    initialized_ = true;
    std::cout << "[YOLOv8] Initialized successfully" << std::endl;
    std::cout << "[YOLOv8] Model: " << model_path_ << std::endl;
    std::cout << "[YOLOv8] Backend: " << backend_ << std::endl;
    std::cout << "[YOLOv8] Device: " << device_id_ << std::endl;
    std::cout << "[YOLOv8] Conf threshold: " << conf_threshold_ << std::endl;
    std::cout << "[YOLOv8] NMS threshold: " << nms_threshold_ << std::endl;
    
    return ALGO_STATUS_SUCCESS;
  }
  
  AlgoStatus infer(const AlgoTensor* input, AlgoDetResult* result) {
    if (!initialized_) {
      return ALGO_STATUS_ERROR_NOT_INITIALIZED;
    }
    
    if (!input || !result) {
      return ALGO_STATUS_ERROR_INVALID_PARAM;
    }
    
    std::cout << "[YOLOv8] Running inference..." << std::endl;
    std::cout << "[YOLOv8] Input shape: [";
    for (int i = 0; i < input->ndim; ++i) {
      std::cout << input->shape[i];
      if (i < input->ndim - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    
    // TODO: 实际推理逻辑
    // 1. 预处理
    // 2. Backend 推理
    // 3. 后处理（NMS）
    
    // 模拟结果
    result->num_boxes = 2;
    result->boxes = new AlgoDetBox[2];
    
    result->boxes[0].x1 = 100.0f;
    result->boxes[0].y1 = 150.0f;
    result->boxes[0].x2 = 300.0f;
    result->boxes[0].y2 = 400.0f;
    result->boxes[0].score = 0.95f;
    result->boxes[0].class_id = 0;
    strcpy(result->boxes[0].class_name, "person");
    
    result->boxes[1].x1 = 200.0f;
    result->boxes[1].y1 = 100.0f;
    result->boxes[1].x2 = 450.0f;
    result->boxes[1].y2 = 350.0f;
    result->boxes[1].score = 0.88f;
    result->boxes[1].class_id = 2;
    strcpy(result->boxes[1].class_name, "car");
    
    result->timestamp = 0;
    
    std::cout << "[YOLOv8] Detected " << result->num_boxes << " objects" << std::endl;
    
    return ALGO_STATUS_SUCCESS;
  }
  
  AlgoStatus deinit() {
    if (!initialized_) {
      return ALGO_STATUS_SUCCESS;
    }
    
    // TODO: 释放 Backend 资源
    
    initialized_ = false;
    std::cout << "[YOLOv8] Deinitialized" << std::endl;
    return ALGO_STATUS_SUCCESS;
  }
  
  bool isInitialized() const { return initialized_; }
  
 private:
  bool initialized_;
  std::string model_path_;
  AlgoBackendType backend_;
  int device_id_;
  
  // 算法参数
  float conf_threshold_;
  float nms_threshold_;
  int input_width_;
  int input_height_;
  
  // Backend 实现（根据类型选择）
  // std::unique_ptr<BackendInterface> backend_impl_;
};

// ============================================================================
// C 接口实现（导出符号）
// ============================================================================

// 静态插件信息
static AlgoBackendType supported_backends[] = {
  ALGO_BACKEND_TENSORRT,
  ALGO_BACKEND_ONNXRUNTIME
};

static AlgoInfo algo_info = {
  "YOLOv8",                                  // name
  "1.0.0",                                   // version
  ALGO_TYPE_DETECTION,                       // type
  "YOLOv8 object detection with multi-backend support",  // description
  "infer-frame",                             // author
  supported_backends,                        // supported_backends
  2                                          // num_backends
};

extern "C" {

const AlgoInfo* AlgoGetInfo() {
  return &algo_info;
}

AlgoHandle AlgoCreate() {
  std::cout << "[YOLOv8] Creating instance..." << std::endl;
  return reinterpret_cast<AlgoHandle>(new YOLOv8Impl());
}

AlgoStatus AlgoInit(AlgoHandle handle, const AlgoInitParam* param) {
  if (!handle) {
    return ALGO_STATUS_ERROR_INVALID_PARAM;
  }
  
  YOLOv8Impl* impl = reinterpret_cast<YOLOv8Impl*>(handle);
  return impl->init(param);
}

AlgoStatus AlgoInferDetection(AlgoHandle handle, const AlgoTensor* input, AlgoDetResult* result) {
  if (!handle) {
    return ALGO_STATUS_ERROR_INVALID_PARAM;
  }
  
  YOLOv8Impl* impl = reinterpret_cast<YOLOv8Impl*>(handle);
  return impl->infer(input, result);
}

AlgoStatus AlgoDeinit(AlgoHandle handle) {
  if (!handle) {
    return ALGO_STATUS_ERROR_INVALID_PARAM;
  }
  
  YOLOv8Impl* impl = reinterpret_cast<YOLOv8Impl*>(handle);
  return impl->deinit();
}

void AlgoDestroy(AlgoHandle handle) {
  if (!handle) {
    return;
  }
  
  std::cout << "[YOLOv8] Destroying instance..." << std::endl;
  YOLOv8Impl* impl = reinterpret_cast<YOLOv8Impl*>(handle);
  delete impl;
}

void AlgoFreeDetResult(AlgoDetResult* result) {
  if (result && result->boxes) {
    delete[] result->boxes;
    result->boxes = nullptr;
    result->num_boxes = 0;
  }
}

}  // extern "C"
