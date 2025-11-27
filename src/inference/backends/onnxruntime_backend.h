#pragma once

#include "inference/backend_interface.h"
#include "utils/one_logger.hpp"

namespace infer_frame {
namespace backend {

/**
 * @brief ONNXRuntime 推理后端
 * 
 * 通用后端，支持 CPU/CUDA/多种硬件加速
 */
class ONNXRuntimeBackend : public BackendInterface {
 public:
  ONNXRuntimeBackend() : initialized_(false) {
    LOG_DEBUG("ONNXRuntimeBackend constructor");
  }
  
  ~ONNXRuntimeBackend() override {
    deinit();
  }
  
  base::Status init(const BackendConfig& config) override {
    if (initialized_) {
      return base::Status::Error(base::StatusCode::kErrorAlreadyInitialized,
                                  "ONNXRuntime backend already initialized");
    }
    
    LOG_INFO("Initializing ONNXRuntime backend...");
    LOG_INFO("Model path: {}", config.model_path);
    LOG_INFO("Device ID: {}", config.device_id);
    
    // TODO: 实际的 ONNXRuntime 初始化代码
    // 1. 创建 Ort::Env
    // 2. 创建 SessionOptions
    // 3. 设置执行提供者 (CPU/CUDA/TensorRT EP)
    // 4. 加载 ONNX 模型
    
    config_ = config;
    initialized_ = true;
    
    LOG_INFO("ONNXRuntime backend initialized successfully");
    return base::Status::OK();
  }
  
  base::Status infer(
      const std::vector<base::Tensor*>& inputs,
      std::vector<base::Tensor*>& outputs) override {
    if (!initialized_) {
      return base::Status::NotInitialized("ONNXRuntime backend not initialized");
    }
    
    LOG_DEBUG("ONNXRuntime infer - inputs: {}, outputs: {}",
              inputs.size(), outputs.size());
    
    // TODO: 实际的推理代码
    // 1. 准备输入 Ort::Value
    // 2. 执行 session->Run()
    // 3. 获取输出结果
    
    return base::Status::OK();
  }
  
  base::Status inferBatch(
      const std::vector<std::vector<base::Tensor*>>& batch_inputs,
      std::vector<std::vector<base::Tensor*>>& batch_outputs) override {
    // TODO: 实现批量推理
    return base::Status::NotImplemented("ONNXRuntime batch inference not implemented yet");
  }
  
  std::vector<base::TensorInfo> getInputInfos() const override {
    // TODO: 从 ONNX session 获取输入信息
    return input_infos_;
  }
  
  std::vector<base::TensorInfo> getOutputInfos() const override {
    // TODO: 从 ONNX session 获取输出信息
    return output_infos_;
  }
  
  base::Status deinit() override {
    if (!initialized_) {
      return base::Status::OK();
    }
    
    LOG_INFO("Deinitializing ONNXRuntime backend...");
    
    // TODO: 释放 ONNXRuntime 资源
    // session_.reset();
    
    initialized_ = false;
    LOG_INFO("ONNXRuntime backend deinitialized");
    
    return base::Status::OK();
  }
  
  BackendType getType() const override {
    return BackendType::kONNXRuntime;
  }
  
  std::string getName() const override {
    return "ONNXRuntime";
  }
  
  bool isInitialized() const override {
    return initialized_;
  }
  
  std::map<std::string, float> getPerformanceStats() const override {
    std::map<std::string, float> stats;
    stats["infer_time_ms"] = 0.0f;  // TODO: 实际统计
    return stats;
  }
  
 private:
  bool initialized_;
  BackendConfig config_;
  std::vector<base::TensorInfo> input_infos_;
  std::vector<base::TensorInfo> output_infos_;
  
  // TODO: ONNXRuntime 相关成员
  // std::unique_ptr<Ort::Env> env_;
  // std::unique_ptr<Ort::Session> session_;
  // Ort::SessionOptions session_options_;
};

}  // namespace backend
}  // namespace infer_frame
