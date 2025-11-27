#include "yolov8_plugin.h"
#include "plugin/plugin_registry.h"
#include "inference/backend_factory.h"

namespace infer_frame {
namespace plugin {

YOLOv8Plugin::YOLOv8Plugin()
    : initialized_(false),
      conf_threshold_(0.25f),
      nms_threshold_(0.45f),
      input_width_(640),
      input_height_(640) {
  LOG_DEBUG("YOLOv8Plugin constructor");
}

YOLOv8Plugin::~YOLOv8Plugin() {
  LOG_DEBUG("YOLOv8Plugin destructor");
  if (initialized_) {
    deinit();
  }
}

AlgoInfo YOLOv8Plugin::getInfo() const {
  AlgoInfo info;
  info.name = "YOLOv8";
  info.version = "1.0.0";
  info.type = AlgoType::kDetection;
  info.description = "YOLOv8 object detection plugin";
  info.author = "Infer-Frame Team";
  info.supported_backends = {
      backend::BackendType::kTensorRT,
      backend::BackendType::kONNXRuntime
  };
  return info;
}

base::Status YOLOv8Plugin::init(
    const std::string& model_path,
    const backend::BackendConfig& backend_config,
    const std::map<std::string, std::string>& algo_params) {
  
  if (initialized_) {
    return base::Status(base::StatusCode::kErrorAlreadyInitialized,
                        "YOLOv8Plugin already initialized");
  }
  
  LOG_INFO("Initializing YOLOv8Plugin...");
  LOG_INFO("Model path: {}", model_path);
  
  // 注意：BackendConfig 中没有 backend_type 字段，需要单独传递
  // LOG_INFO("Backend type: {}", static_cast<int>(backend_config.backend_type));
  
  model_path_ = model_path;
  
  // 解析算法参数
  if (algo_params.count("conf_threshold")) {
    conf_threshold_ = std::stof(algo_params.at("conf_threshold"));
  }
  if (algo_params.count("nms_threshold")) {
    nms_threshold_ = std::stof(algo_params.at("nms_threshold"));
  }
  if (algo_params.count("input_width")) {
    input_width_ = std::stoi(algo_params.at("input_width"));
  }
  if (algo_params.count("input_height")) {
    input_height_ = std::stoi(algo_params.at("input_height"));
  }
  
  LOG_INFO("YOLO params - conf: {}, nms: {}, size: {}x{}",
           conf_threshold_, nms_threshold_, input_width_, input_height_);
  
  // 创建后端（从 algo_params 中获取 backend_type）
  auto& factory = backend::BackendFactory::getInstance();
  
  // 默认使用 TensorRT
  backend::BackendType backend_type = backend::BackendType::kTensorRT;
  if (algo_params.count("backend_type")) {
    // 可以从参数中指定后端类型
    backend_type = static_cast<backend::BackendType>(std::stoi(algo_params.at("backend_type")));
  }
  
  backend_ = factory.createBackend(backend_type);
  if (!backend_) {
    return base::Status(base::StatusCode::kErrorBackendNotSupported,
                        "Failed to create backend");
  }
  
  // 初始化后端
  auto status = backend_->init(backend_config);
  if (!status.ok()) {
    return status;
  }
  
  initialized_ = true;
  LOG_INFO("YOLOv8Plugin initialized successfully");
  
  return base::Status::OK();
}

base::Status YOLOv8Plugin::infer(
    std::vector<base::Tensor*>& inputs,
    std::vector<base::Tensor*>& outputs) {
  
  if (!initialized_) {
    return base::Status(base::StatusCode::kErrorNotInitialized,
                        "YOLOv8Plugin not initialized");
  }
  
  LOG_DEBUG("YOLOv8 infer - inputs: {}, outputs: {}",
            inputs.size(), outputs.size());
  
  // TODO: 实现实际的 YOLO 推理逻辑
  // 1. 预处理
  // 2. 后端推理
  // 3. 后处理（解析检测框、NMS 等）
  
  // 占位实现
  if (backend_) {
    return backend_->infer(inputs, outputs);
  }
  
  return base::Status::OK();
}

base::Status YOLOv8Plugin::inferBatch(
    const std::vector<std::vector<base::Tensor*>>& batch_inputs,
    std::vector<std::vector<base::Tensor*>>& batch_outputs) {
  
  if (!initialized_) {
    return base::Status(base::StatusCode::kErrorNotInitialized,
                        "YOLOv8Plugin not initialized");
  }
  
  LOG_DEBUG("YOLOv8 inferBatch - batch size: {}", batch_inputs.size());
  
  // TODO: 实现批量推理
  // 占位实现：逐帧推理
  for (size_t i = 0; i < batch_inputs.size(); ++i) {
    auto inputs = const_cast<std::vector<base::Tensor*>&>(batch_inputs[i]);
    std::vector<base::Tensor*> outputs;
    auto status = infer(inputs, outputs);
    if (!status.ok()) {
      return status;
    }
    batch_outputs.push_back(outputs);
  }
  
  return base::Status::OK();
}

base::Status YOLOv8Plugin::deinit() {
  if (!initialized_) {
    return base::Status::OK();
  }
  
  LOG_INFO("Deinitializing YOLOv8Plugin...");
  
  if (backend_) {
    backend_->deinit();
    backend_.reset();
  }
  
  initialized_ = false;
  LOG_INFO("YOLOv8Plugin deinitialized");
  
  return base::Status::OK();
}

base::Status YOLOv8Plugin::preprocess(base::Tensor* input, base::Tensor* output) {
  // TODO: 实现预处理
  // - Resize 到 input_width_ x input_height_
  // - 归一化到 [0, 1]
  // - HWC -> CHW
  // - BGR -> RGB (if needed)
  return base::Status::OK();
}

base::Status YOLOv8Plugin::postprocess(base::Tensor* input, base::Tensor* output) {
  // TODO: 实现后处理
  // - 解析检测结果
  // - 置信度过滤
  // - NMS
  // - 坐标映射回原图
  return base::Status::OK();
}

}  // namespace plugin
}  // namespace infer_frame

// 注册插件
REGISTER_ALGO_PLUGIN(infer_frame::plugin::YOLOv8Plugin)
