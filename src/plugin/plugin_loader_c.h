#pragma once

#include "plugin/algo_plugin_interface.h"
#include "utils/one_logger.hpp"

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <dlfcn.h>

namespace infer_frame {
namespace plugin {

/**
 * @brief C 风格插件加载器（参考 VSE loadLib）
 * 
 * 特点：
 * 1. 使用 dlopen/dlsym 加载 C 函数
 * 2. 支持独立编译的插件
 * 3. Backend 由插件内部管理
 */
class PluginLoaderC {
 public:
  PluginLoaderC() = default;
  ~PluginLoaderC();
  
  // 禁止拷贝和赋值
  PluginLoaderC(const PluginLoaderC&) = delete;
  PluginLoaderC& operator=(const PluginLoaderC&) = delete;
  
  /**
   * @brief 加载插件
   * @param plugin_path 插件 .so 文件路径
   * @return 是否成功
   */
  bool loadPlugin(const std::string& plugin_path);
  
  /**
   * @brief 卸载插件
   * @param plugin_name 插件名称
   * @return 是否成功
   */
  bool unloadPlugin(const std::string& plugin_name);
  
  /**
   * @brief 卸载所有插件
   */
  void unloadAll();
  
  /**
   * @brief 获取已加载的插件列表
   */
  std::vector<std::string> getLoadedPlugins() const;
  
  /**
   * @brief 获取插件信息
   * @param plugin_name 插件名称
   * @return 插件信息，未找到返回 nullptr
   */
  const AlgoInfo* getPluginInfo(const std::string& plugin_name);
  
  /**
   * @brief 创建算法实例
   * @param plugin_name 插件名称
   * @return 算法句柄，失败返回 nullptr
   */
  AlgoHandle createAlgoInstance(const std::string& plugin_name);
  
  /**
   * @brief 初始化算法
   * @param handle 算法句柄
   * @param plugin_name 插件名称
   * @param param 初始化参数
   * @return 状态码
   */
  AlgoStatus initAlgo(AlgoHandle handle, const std::string& plugin_name, const AlgoInitParam* param);
  
  /**
   * @brief 执行推理
   * @param handle 算法句柄
   * @param plugin_name 插件名称
   * @param input 输入 Tensor
   * @param result 检测结果
   * @return 状态码
   */
  AlgoStatus inferDetection(AlgoHandle handle, const std::string& plugin_name,
                           const AlgoTensor* input, AlgoDetResult* result);
  
  /**
   * @brief 反初始化算法
   * @param handle 算法句柄
   * @param plugin_name 插件名称
   * @return 状态码
   */
  AlgoStatus deinitAlgo(AlgoHandle handle, const std::string& plugin_name);
  
  /**
   * @brief 销毁算法实例
   * @param handle 算法句柄
   * @param plugin_name 插件名称
   */
  void destroyAlgoInstance(AlgoHandle handle, const std::string& plugin_name);
  
 private:
  /**
   * @brief 插件句柄信息
   */
  struct PluginHandle {
    void* dl_handle;        // dlopen 返回的句柄
    std::string path;       // 插件文件路径
    
    // 函数指针（参考 VSE）
    const AlgoInfo* (*getInfo)();
    AlgoHandle (*create)();
    AlgoStatus (*init)(AlgoHandle, const AlgoInitParam*);
    AlgoStatus (*inferDetection)(AlgoHandle, const AlgoTensor*, AlgoDetResult*);
    AlgoStatus (*deinit)(AlgoHandle);
    void (*destroy)(AlgoHandle);
    void (*freeDetResult)(AlgoDetResult*);
  };
  
  std::map<std::string, PluginHandle> loaded_plugins_;
  mutable std::mutex mutex_;
  
  /**
   * @brief 检查文件是否存在
   */
  bool fileExists(const std::string& path) const;
  
  /**
   * @brief 从 .so 文件中加载函数指针
   */
  bool loadFunctions(PluginHandle& handle);
};

// ============================================================================
// 内联实现
// ============================================================================

inline PluginLoaderC::~PluginLoaderC() {
  unloadAll();
}

inline bool PluginLoaderC::loadPlugin(const std::string& plugin_path) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  if (!fileExists(plugin_path)) {
    LOG_ERROR("Plugin file not found: {}", plugin_path);
    return false;
  }
  
  LOG_INFO("Loading plugin from: {}", plugin_path);
  
  PluginHandle handle;
  handle.path = plugin_path;
  
  // 打开动态库
  handle.dl_handle = dlopen(plugin_path.c_str(), RTLD_NOW | RTLD_LOCAL);
  if (!handle.dl_handle) {
    LOG_ERROR("Failed to load plugin: {}", dlerror());
    return false;
  }
  
  // 加载函数指针
  if (!loadFunctions(handle)) {
    dlclose(handle.dl_handle);
    return false;
  }
  
  // 获取插件信息
  const AlgoInfo* info = handle.getInfo();
  if (!info) {
    LOG_ERROR("Failed to get plugin info");
    dlclose(handle.dl_handle);
    return false;
  }
  
  std::string plugin_name = info->name;
  loaded_plugins_[plugin_name] = handle;
  
  LOG_INFO("Plugin loaded successfully: {} v{}", info->name, info->version);
  return true;
}

inline bool PluginLoaderC::unloadPlugin(const std::string& plugin_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = loaded_plugins_.find(plugin_name);
  if (it == loaded_plugins_.end()) {
    return false;
  }
  
  if (it->second.dl_handle) {
    dlclose(it->second.dl_handle);
  }
  
  loaded_plugins_.erase(it);
  LOG_INFO("Plugin unloaded: {}", plugin_name);
  return true;
}

inline void PluginLoaderC::unloadAll() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  for (auto& pair : loaded_plugins_) {
    if (pair.second.dl_handle) {
      dlclose(pair.second.dl_handle);
    }
  }
  
  loaded_plugins_.clear();
  LOG_INFO("All plugins unloaded");
}

inline std::vector<std::string> PluginLoaderC::getLoadedPlugins() const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::vector<std::string> names;
  for (const auto& pair : loaded_plugins_) {
    names.push_back(pair.first);
  }
  return names;
}

inline const AlgoInfo* PluginLoaderC::getPluginInfo(const std::string& plugin_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = loaded_plugins_.find(plugin_name);
  if (it == loaded_plugins_.end()) {
    return nullptr;
  }
  
  return it->second.getInfo();
}

inline AlgoHandle PluginLoaderC::createAlgoInstance(const std::string& plugin_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = loaded_plugins_.find(plugin_name);
  if (it == loaded_plugins_.end()) {
    LOG_ERROR("Plugin not found: {}", plugin_name);
    return nullptr;
  }
  
  return it->second.create();
}

inline AlgoStatus PluginLoaderC::initAlgo(AlgoHandle handle, const std::string& plugin_name,
                                         const AlgoInitParam* param) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = loaded_plugins_.find(plugin_name);
  if (it == loaded_plugins_.end()) {
    return ALGO_STATUS_ERROR_INVALID_PARAM;
  }
  
  return it->second.init(handle, param);
}

inline AlgoStatus PluginLoaderC::inferDetection(AlgoHandle handle, const std::string& plugin_name,
                                                const AlgoTensor* input, AlgoDetResult* result) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = loaded_plugins_.find(plugin_name);
  if (it == loaded_plugins_.end()) {
    return ALGO_STATUS_ERROR_INVALID_PARAM;
  }
  
  return it->second.inferDetection(handle, input, result);
}

inline AlgoStatus PluginLoaderC::deinitAlgo(AlgoHandle handle, const std::string& plugin_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = loaded_plugins_.find(plugin_name);
  if (it == loaded_plugins_.end()) {
    return ALGO_STATUS_ERROR_INVALID_PARAM;
  }
  
  return it->second.deinit(handle);
}

inline void PluginLoaderC::destroyAlgoInstance(AlgoHandle handle, const std::string& plugin_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = loaded_plugins_.find(plugin_name);
  if (it == loaded_plugins_.end()) {
    return;
  }
  
  it->second.destroy(handle);
}

inline bool PluginLoaderC::fileExists(const std::string& path) const {
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
}

inline bool PluginLoaderC::loadFunctions(PluginHandle& handle) {
  // 加载所有必需的函数（参考 VSE）
  handle.getInfo = (decltype(handle.getInfo))dlsym(handle.dl_handle, "AlgoGetInfo");
  handle.create = (decltype(handle.create))dlsym(handle.dl_handle, "AlgoCreate");
  handle.init = (decltype(handle.init))dlsym(handle.dl_handle, "AlgoInit");
  handle.inferDetection = (decltype(handle.inferDetection))dlsym(handle.dl_handle, "AlgoInferDetection");
  handle.deinit = (decltype(handle.deinit))dlsym(handle.dl_handle, "AlgoDeinit");
  handle.destroy = (decltype(handle.destroy))dlsym(handle.dl_handle, "AlgoDestroy");
  handle.freeDetResult = (decltype(handle.freeDetResult))dlsym(handle.dl_handle, "AlgoFreeDetResult");
  
  const char* dlsym_error = dlerror();
  if (dlsym_error) {
    LOG_ERROR("Cannot load symbols: {}", dlsym_error);
    return false;
  }
  
  if (!handle.getInfo || !handle.create || !handle.init || 
      !handle.inferDetection || !handle.deinit || !handle.destroy) {
    LOG_ERROR("Missing required functions in plugin");
    return false;
  }
  
  return true;
}

}  // namespace plugin
}  // namespace infer_frame
