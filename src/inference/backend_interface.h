#pragma once

#include "inference/base/status.h"
#include "inference/base/types.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace infer_frame {
namespace backend {

// 使用 base 命名空间中的类型
using base::BackendType;
using base::BackendConfig;
using base::TensorInfo;

/**
 * @brief 推理后端抽象接口
 * 
 * 所有推理后端（TensorRT, ONNXRuntime, RKNN 等）必须实现此接口
 * 确保算法代码与具体后端解耦
 */
class BackendInterface {
 public:
  virtual ~BackendInterface() = default;
  
  /**
   * @brief 初始化后端
   * @param config 后端配置
   * @return 状态码
   */
  virtual base::Status init(const BackendConfig& config) = 0;
  
  /**
   * @brief 单次推理
   * @param inputs 输入 Tensor 列表
   * @param outputs 输出 Tensor 列表（由调用者预分配或由函数内部分配）
   * @return 状态码
   */
  virtual base::Status infer(
      const std::vector<base::Tensor*>& inputs,
      std::vector<base::Tensor*>& outputs) = 0;
  
  /**
   * @brief 批量推理
   * @param batch_inputs 批量输入
   * @param batch_outputs 批量输出
   * @return 状态码
   */
  virtual base::Status inferBatch(
      const std::vector<std::vector<base::Tensor*>>& batch_inputs,
      std::vector<std::vector<base::Tensor*>>& batch_outputs) = 0;
  
  /**
   * @brief 获取输入 Tensor 信息
   * @return 输入 Tensor 信息列表
   */
  virtual std::vector<base::TensorInfo> getInputInfos() const = 0;
  
  /**
   * @brief 获取输出 Tensor 信息
   * @return 输出 Tensor 信息列表
   */
  virtual std::vector<base::TensorInfo> getOutputInfos() const = 0;
  
  /**
   * @brief 反初始化，释放资源
   * @return 状态码
   */
  virtual base::Status deinit() = 0;
  
  /**
   * @brief 获取后端类型
   * @return 后端类型
   */
  virtual BackendType getType() const = 0;
  
  /**
   * @brief 获取后端名称
   * @return 后端名称字符串
   */
  virtual std::string getName() const = 0;
  
  /**
   * @brief 检查后端是否已初始化
   * @return true 如果已初始化
   */
  virtual bool isInitialized() const = 0;
  
  /**
   * @brief 获取性能统计信息（可选）
   * @return 性能统计数据
   */
  virtual std::map<std::string, float> getPerformanceStats() const {
    return {};  // 默认返回空
  }
};

/**
 * @brief 后端类型转字符串
 */
inline std::string backendTypeToString(BackendType type) {
  switch (type) {
    case BackendType::kTensorRT: return "TensorRT";
    case BackendType::kONNXRuntime: return "ONNXRuntime";
    case BackendType::kRKNN: return "RKNN";
    case BackendType::kSophon: return "Sophon";
    case BackendType::kOpenVINO: return "OpenVINO";
    case BackendType::kPaddleInference: return "PaddleInference";
    case BackendType::kMNN: return "MNN";
    default: return "Unknown";
  }
}

/**
 * @brief 字符串转后端类型
 */
inline BackendType stringToBackendType(const std::string& type_str) {
  if (type_str == "TensorRT") return BackendType::kTensorRT;
  if (type_str == "ONNXRuntime") return BackendType::kONNXRuntime;
  if (type_str == "RKNN") return BackendType::kRKNN;
  if (type_str == "Sophon") return BackendType::kSophon;
  if (type_str == "OpenVINO") return BackendType::kOpenVINO;
  if (type_str == "PaddleInference") return BackendType::kPaddleInference;
  if (type_str == "MNN") return BackendType::kMNN;
  return BackendType::kUnknown;
}

}  // namespace backend
}  // namespace infer_frame
