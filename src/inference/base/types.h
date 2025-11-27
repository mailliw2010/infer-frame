#pragma once

/**
 * @file types.h
 * @brief 对接 NNDeploy 的类型定义
 * 
 * 直接使用 NNDeploy 的 Tensor、Device 等类型，
 * 避免重复造轮子，获得 NNDeploy 的：
 * - 多后端支持（TensorRT/ONNX/OpenVINO/MNN/ncnn 等）
 * - 内存池管理
 * - 零拷贝优化
 * - 并行推理能力
 */

// NNDeploy 核心类型
#include "nndeploy/device/tensor.h"
#include "nndeploy/device/device.h"
#include "nndeploy/device/buffer.h"
#include "nndeploy/base/common.h"
#include "nndeploy/base/status.h"

#include <vector>
#include <string>
#include <memory>
#include <map>

namespace infer_frame {
namespace base {

// ============================================================================
// 直接使用 NNDeploy 的类型
// ============================================================================

// Tensor 类型别名
using Tensor = nndeploy::device::Tensor;
using TensorDesc = nndeploy::device::TensorDesc;

// Device 类型别名
using Device = nndeploy::device::Device;
using Buffer = nndeploy::device::Buffer;

// 数据类型别名
using DataType = nndeploy::base::DataType;
using DeviceType = nndeploy::base::DeviceType;
// 注意：Status 和 StatusCode 使用我们自己的定义（在 status.h 中），不使用 NNDeploy 的
using IntVector = nndeploy::base::IntVector;
using SizeVector = nndeploy::base::SizeVector;

// NNDeploy 状态码（如需要可在内部使用）
namespace nndeploy_status {
  using Status = nndeploy::base::Status;
  using StatusCode = nndeploy::base::StatusCode;
  constexpr auto kStatusCodeOk = nndeploy::base::kStatusCodeOk;
  constexpr auto kStatusCodeErrorInvalidParam = nndeploy::base::kStatusCodeErrorInvalidParam;
  constexpr auto kStatusCodeErrorNullParam = nndeploy::base::kStatusCodeErrorNullParam;
}

// 设备类型常量
constexpr auto kDeviceTypeCodeCpu = nndeploy::base::kDeviceTypeCodeCpu;
constexpr auto kDeviceTypeCodeCuda = nndeploy::base::kDeviceTypeCodeCuda;
constexpr auto kDeviceTypeCodeArm = nndeploy::base::kDeviceTypeCodeArm;
constexpr auto kDeviceTypeCodeX86 = nndeploy::base::kDeviceTypeCodeX86;

// 数据类型常量
constexpr auto kDataTypeCodeFp = nndeploy::base::kDataTypeCodeFp;
constexpr auto kDataTypeCodeInt = nndeploy::base::kDataTypeCodeInt;
constexpr auto kDataTypeCodeUint = nndeploy::base::kDataTypeCodeUint;

// ============================================================================
// Backend 配置（保持兼容）
// ============================================================================

/**
 * @brief Tensor 元信息（描述张量的形状、类型等）
 */
struct TensorInfo {
  std::string name;           // Tensor 名称
  std::vector<int> shape;     // Tensor 形状（使用 int 匹配 NNDeploy）
  DataType dtype;             // 数据类型
  
  TensorInfo() = default;
  TensorInfo(const std::string& n, const std::vector<int>& s, DataType dt)
    : name(n), shape(s), dtype(dt) {}
  
  // 从 NNDeploy TensorDesc 转换
  static TensorInfo fromTensorDesc(const std::string& name, const TensorDesc& desc) {
    TensorInfo info;
    info.name = name;
    info.shape = desc.shape_;
    info.dtype = desc.data_type_;
    return info;
  }
  
  // 转换到 NNDeploy TensorDesc
  TensorDesc toTensorDesc() const {
    TensorDesc desc;
    desc.shape_ = shape;
    desc.data_type_ = dtype;
    return desc;
  }
};

/**
 * @brief Backend 类型枚举（映射到 NNDeploy InferenceType）
 */
enum class BackendType {
  kTensorRT = 0,
  kONNXRuntime = 1,
  kOpenVINO = 2,
  kMNN = 3,
  kTNN = 4,
  kNCNN = 5,
  kCoreML = 6,
  kRKNN = 7,
  kAscendCL = 8,
  kSophon = 9,
  kPaddleInference = 10,
  kUnknown = 99
};

/**
 * @brief Backend 配置
 */
struct BackendConfig {
  std::string model_path;
  BackendType backend_type;
  int device_id = 0;
  std::map<std::string, std::string> options;
};

// ============================================================================
// 辅助函数：Backend 类型转换
// ============================================================================

inline nndeploy::base::InferenceType toNNDeployInferenceType(BackendType type) {
  switch (type) {
    case BackendType::kTensorRT:
      return nndeploy::base::kInferenceTypeTensorRt;
    case BackendType::kONNXRuntime:
      return nndeploy::base::kInferenceTypeOnnxRuntime;
    case BackendType::kOpenVINO:
      return nndeploy::base::kInferenceTypeOpenVino;
    case BackendType::kMNN:
      return nndeploy::base::kInferenceTypeMnn;
    case BackendType::kTNN:
      return nndeploy::base::kInferenceTypeTnn;
    case BackendType::kNCNN:
      return nndeploy::base::kInferenceTypeNcnn;
    case BackendType::kCoreML:
      return nndeploy::base::kInferenceTypeCoreML;
    case BackendType::kRKNN:
      return nndeploy::base::kInferenceTypeRknn;
    case BackendType::kAscendCL:
      return nndeploy::base::kInferenceTypeAscendCL;
    default:
      return nndeploy::base::kInferenceTypeDefault;
  }
}

}  // namespace base
}  // namespace infer_frame
