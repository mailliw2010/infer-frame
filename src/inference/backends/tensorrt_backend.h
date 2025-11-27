#pragma once

#include "inference/backend_interface.h"
#include "utils/one_logger.hpp"

// 直接暴露给用户使用 NNDeploy，不做过多包装
namespace infer_frame {
namespace backend {

/**
 * @brief TensorRT 推理后端 (简化版)
 * 
 * 直接使用 NNDeploy，保持简单
 */
class TensorRTBackend : public BackendInterface {
 public:
  TensorRTBackend() {
    LOG_DEBUG("TensorRTBackend constructor");
  }
  
  ~TensorRTBackend() override = default;
  
  base::Status init(const BackendConfig& config) override {
    LOG_INFO("Initializing TensorRT backend...");
    LOG_INFO("Model path: {}", config.model_path);
    LOG_INFO("TensorRT backend initialized (stub implementation)");
    LOG_INFO("Please use NNDeploy TensorRT API directly");
    return base::Status::OK();
  }
  
  base::Status infer(
      const std::vector<base::Tensor*>& inputs,
      std::vector<base::Tensor*>& outputs) override {
    LOG_DEBUG("TensorRT infer called");
    return base::Status::OK();
  }
  
  base::Status inferBatch(
      const std::vector<std::vector<base::Tensor*>>& batch_inputs,
      std::vector<std::vector<base::Tensor*>>& batch_outputs) override {
    return base::Status::OK();
  }
  
  std::vector<base::TensorInfo> getInputInfos() const override {
    return {};
  }
  
  std::vector<base::TensorInfo> getOutputInfos() const override {
    return {};
  }
  
  base::Status deinit() override {
    LOG_INFO("TensorRT backend deinitialized");
    return base::Status::OK();
  }
  
  BackendType getType() const override {
    return BackendType::kTensorRT;
  }
  
  std::string getName() const override {
    return "TensorRT";
  }
  
  bool isInitialized() const override {
    return true;
  }
  
  std::map<std::string, float> getPerformanceStats() const override {
    return {};
  }
};

}  // namespace backend
}  // namespace infer_frame
