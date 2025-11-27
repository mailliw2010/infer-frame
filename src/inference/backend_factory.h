#pragma once

#include "inference/backend_interface.h"
#include "utils/one_logger.hpp"
#include <functional>
#include <mutex>

namespace infer_frame {
namespace backend {

// 使用 base 命名空间中的类型
using base::BackendType;
using base::BackendConfig;

/**
 * @brief Backend 工厂类 - 用于创建不同类型的推理后端
 * 
 * 使用工厂模式创建后端实例，支持运行时注册新后端
 */
class BackendFactory {
 public:
  using BackendCreator = std::function<std::shared_ptr<BackendInterface>()>;
  
  /**
   * @brief 获取工厂单例
   */
  static BackendFactory& getInstance() {
    static BackendFactory instance;
    return instance;
  }
  
  /**
   * @brief 注册后端创建函数
   * @param type 后端类型
   * @param creator 创建函数
   */
  void registerBackend(BackendType type, BackendCreator creator) {
    std::lock_guard<std::mutex> lock(mutex_);
    creators_[type] = creator;
    LOG_INFO("Registered backend: {}", backendTypeToString(type));
  }
  
  /**
   * @brief 创建后端实例
   * @param type 后端类型
   * @return 后端实例指针，如果类型不支持则返回 nullptr
   */
  std::shared_ptr<BackendInterface> createBackend(BackendType type) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = creators_.find(type);
    if (it == creators_.end()) {
      LOG_ERROR("Backend type not supported: {}", backendTypeToString(type));
      return nullptr;
    }
    
    LOG_INFO("Creating backend: {}", backendTypeToString(type));
    return it->second();
  }
  
  /**
   * @brief 从配置创建后端
   * @param config 后端配置
   * @return 后端实例指针
   */
  std::shared_ptr<BackendInterface> createBackend(const BackendConfig& config) {
    auto backend = createBackend(config.backend_type);
    if (!backend) {
      return nullptr;
    }
    
    // 初始化后端
    auto status = backend->init(config);
    if (!status.ok()) {
      LOG_ERROR("Failed to initialize backend {}: {}", 
                backendTypeToString(config.backend_type), status.message());
      return nullptr;
    }
    
    return backend;
  }
  
  /**
   * @brief 检查后端是否已注册
   * @param type 后端类型
   * @return true 如果已注册
   */
  bool isBackendSupported(BackendType type) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return creators_.find(type) != creators_.end();
  }
  
  /**
   * @brief 获取所有已注册的后端类型
   * @return 后端类型列表
   */
  std::vector<BackendType> getSupportedBackends() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<BackendType> types;
    for (const auto& pair : creators_) {
      types.push_back(pair.first);
    }
    return types;
  }
  
  // 禁止拷贝和赋值
  BackendFactory(const BackendFactory&) = delete;
  BackendFactory& operator=(const BackendFactory&) = delete;
  
 private:
  BackendFactory() {
    LOG_INFO("BackendFactory initialized");
  }
  
  ~BackendFactory() = default;
  
  std::map<BackendType, BackendCreator> creators_;
  mutable std::mutex mutex_;
};

/**
 * @brief 后端注册辅助宏
 * 
 * 使用示例:
 * REGISTER_BACKEND(TensorRT, TensorRTBackend);
 */
#define REGISTER_BACKEND(backend_type, backend_class) \
  namespace { \
  struct backend_class##Registrar { \
    backend_class##Registrar() { \
      infer_frame::backend::BackendFactory::getInstance().registerBackend( \
          infer_frame::backend::BackendType::k##backend_type, \
          []() -> std::shared_ptr<infer_frame::backend::BackendInterface> { \
            return std::make_shared<infer_frame::backend::backend_class>(); \
          }); \
    } \
  }; \
  static backend_class##Registrar g_##backend_class##_registrar; \
  }

/**
 * @brief 初始化所有后端注册
 * 
 * 显式调用以确保后端注册代码被链接和执行
 */
void initializeBackends();

}  // namespace backend
}  // namespace infer_frame
