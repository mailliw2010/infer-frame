#pragma once

#include "plugin/algo_plugin_base.h"
#include "inference/backend_interface.h"
#include "utils/one_logger.hpp"
#include <memory>

namespace infer_frame {
namespace plugin {

/**
 * @brief YOLOv8 目标检测插件
 * 
 * 这是一个示例插件，演示如何实现算法插件接口。
 * 实际的 YOLO 检测逻辑需要根据具体需求实现。
 */
class YOLOv8Plugin : public AlgoPluginBase {
 public:
  YOLOv8Plugin();
  ~YOLOv8Plugin() override;
  
  // 实现 AlgoPluginBase 接口
  AlgoInfo getInfo() const override;
  
  base::Status init(
      const std::string& model_path,
      const backend::BackendConfig& backend_config,
      const std::map<std::string, std::string>& algo_params) override;
  
  base::Status infer(
      std::vector<base::Tensor*>& inputs,
      std::vector<base::Tensor*>& outputs) override;
  
  base::Status inferBatch(
      const std::vector<std::vector<base::Tensor*>>& batch_inputs,
      std::vector<std::vector<base::Tensor*>>& batch_outputs) override;
  
  base::Status deinit() override;
  
  bool isInitialized() const override { return initialized_; }
  
 private:
  bool initialized_;
  std::shared_ptr<backend::BackendInterface> backend_;
  std::string model_path_;
  
  // YOLO 参数
  float conf_threshold_;  // 置信度阈值
  float nms_threshold_;   // NMS 阈值
  int input_width_;       // 输入宽度
  int input_height_;      // 输入高度
  
  /**
   * @brief 预处理：图像 resize、归一化等
   */
  base::Status preprocess(base::Tensor* input, base::Tensor* output);
  
  /**
   * @brief 后处理：解析检测结果、NMS 等
   */
  base::Status postprocess(base::Tensor* input, base::Tensor* output);
};

}  // namespace plugin
}  // namespace infer_frame
