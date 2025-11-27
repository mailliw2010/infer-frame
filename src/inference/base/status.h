#pragma once

#include <string>
#include <sstream>

namespace infer_frame {
namespace base {

/**
 * @brief 状态码枚举
 */
enum class StatusCode {
  kSuccess = 0,           // 成功
  kErrorInvalidParam,     // 无效参数
  kErrorNotInitialized,   // 未初始化
  kErrorAlreadyInitialized, // 已经初始化
  kErrorOutOfMemory,      // 内存不足
  kErrorFileNotFound,     // 文件不存在
  kErrorModelLoad,        // 模型加载失败
  kErrorInference,        // 推理失败
  kErrorNotImplemented,   // 功能未实现
  kErrorBackendNotSupported, // 后端不支持
  kErrorDeviceNotAvailable,  // 设备不可用
  kErrorTimeout,          // 超时
  kErrorUnknown          // 未知错误
};

/**
 * @brief 状态类 - 用于返回函数执行结果
 */
class Status {
 public:
  // 构造函数
  Status() : code_(StatusCode::kSuccess), message_("") {}
  
  Status(StatusCode code, const std::string& message = "")
      : code_(code), message_(message) {}
  
  // 静态工厂方法
  static Status OK() {
    return Status(StatusCode::kSuccess, "Success");
  }
  
  static Status Error(StatusCode code, const std::string& message) {
    return Status(code, message);
  }
  
  static Status InvalidParam(const std::string& message = "Invalid parameter") {
    return Status(StatusCode::kErrorInvalidParam, message);
  }
  
  static Status NotInitialized(const std::string& message = "Not initialized") {
    return Status(StatusCode::kErrorNotInitialized, message);
  }
  
  static Status ModelLoadError(const std::string& message = "Model load failed") {
    return Status(StatusCode::kErrorModelLoad, message);
  }
  
  static Status InferenceError(const std::string& message = "Inference failed") {
    return Status(StatusCode::kErrorInference, message);
  }
  
  static Status NotImplemented(const std::string& message = "Not implemented") {
    return Status(StatusCode::kErrorNotImplemented, message);
  }
  
  // 判断是否成功
  bool ok() const { return code_ == StatusCode::kSuccess; }
  bool isOk() const { return ok(); }
  
  // 获取状态码
  StatusCode code() const { return code_; }
  
  // 获取错误消息
  const std::string& message() const { return message_; }
  
  // 转换为字符串
  std::string toString() const {
    std::stringstream ss;
    ss << "[" << codeToString(code_) << "] " << message_;
    return ss.str();
  }
  
  // 重载操作符
  explicit operator bool() const { return ok(); }
  
 private:
  StatusCode code_;
  std::string message_;
  
  // 状态码转字符串
  static std::string codeToString(StatusCode code) {
    switch (code) {
      case StatusCode::kSuccess: return "Success";
      case StatusCode::kErrorInvalidParam: return "InvalidParam";
      case StatusCode::kErrorNotInitialized: return "NotInitialized";
      case StatusCode::kErrorAlreadyInitialized: return "AlreadyInitialized";
      case StatusCode::kErrorOutOfMemory: return "OutOfMemory";
      case StatusCode::kErrorFileNotFound: return "FileNotFound";
      case StatusCode::kErrorModelLoad: return "ModelLoadError";
      case StatusCode::kErrorInference: return "InferenceError";
      case StatusCode::kErrorNotImplemented: return "NotImplemented";
      case StatusCode::kErrorBackendNotSupported: return "BackendNotSupported";
      case StatusCode::kErrorDeviceNotAvailable: return "DeviceNotAvailable";
      case StatusCode::kErrorTimeout: return "Timeout";
      case StatusCode::kErrorUnknown: return "Unknown";
      default: return "UnknownCode";
    }
  }
};

}  // namespace base
}  // namespace infer_frame
