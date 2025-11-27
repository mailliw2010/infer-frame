#pragma once

#include "plugin/algo_plugin_base.h"
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
 * @brief 插件加载器
 * 
 * 负责动态加载算法插件 .so 文件，管理插件生命周期。
 * 支持插件目录扫描、加载、卸载等功能。
 * 
 * 使用示例:
 * @code
 * PluginLoader loader;
 * auto plugin = loader.loadPlugin("/path/to/libyolov8_plugin.so");
 * if (plugin) {
 *   LOG_INFO("Plugin loaded: {}", plugin->getInfo().name);
 * }
 * @endcode
 */
class PluginLoader {
 public:
  PluginLoader() = default;
  ~PluginLoader();
  
  // 禁止拷贝和赋值
  PluginLoader(const PluginLoader&) = delete;
  PluginLoader& operator=(const PluginLoader&) = delete;
  
  /**
   * @brief 从文件路径加载插件
   * 
   * @param plugin_path 插件 .so 文件的绝对路径
   * @return 插件实例智能指针，失败返回 nullptr
   */
  std::shared_ptr<AlgoPluginBase> loadPlugin(const std::string& plugin_path);
  
  /**
   * @brief 扫描目录下的所有插件文件
   * 
   * @param plugin_dir 插件目录路径
   * @return 找到的插件文件路径列表
   */
  std::vector<std::string> scanPlugins(const std::string& plugin_dir);
  
  /**
   * @brief 批量加载目录下的所有插件
   * 
   * @param plugin_dir 插件目录路径
   * @return 成功加载的插件列表
   */
  std::vector<std::shared_ptr<AlgoPluginBase>> loadPluginsFromDir(
      const std::string& plugin_dir);
  
  /**
   * @brief 卸载指定插件
   * 
   * @param plugin_name 插件名称
   * @return 是否成功卸载
   */
  bool unloadPlugin(const std::string& plugin_name);
  
  /**
   * @brief 卸载所有插件
   */
  void unloadAll();
  
  /**
   * @brief 获取已加载的插件列表
   * @return 插件名称列表
   */
  std::vector<std::string> getLoadedPlugins() const;
  
  /**
   * @brief 根据名称获取插件
   * 
   * @param plugin_name 插件名称
   * @return 插件实例，未找到返回 nullptr
   */
  std::shared_ptr<AlgoPluginBase> getPlugin(const std::string& plugin_name);
  
 private:
  /**
   * @brief 插件句柄信息
   */
  struct PluginHandle {
    void* dl_handle;                           // dlopen 返回的句柄
    std::shared_ptr<AlgoPluginBase> instance;  // 插件实例
    std::string path;                          // 插件文件路径
  };
  
  std::map<std::string, PluginHandle> loaded_plugins_;  // 已加载的插件
  mutable std::mutex mutex_;                            // 线程安全锁
  
  /**
   * @brief 检查文件是否为有效的插件文件
   * 
   * @param filename 文件名
   * @return 是否为有效插件
   */
  bool isValidPluginFile(const std::string& filename) const;
  
  /**
   * @brief 从 .so 文件中提取创建函数
   * 
   * @param dl_handle dlopen 句柄
   * @return 创建函数指针，失败返回 nullptr
   */
  CreateAlgoPluginFunc getCreateFunction(void* dl_handle);
};

// PluginLoader 实现
inline PluginLoader::~PluginLoader() {
  unloadAll();
}

inline std::shared_ptr<AlgoPluginBase> PluginLoader::loadPlugin(
    const std::string& plugin_path) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  LOG_INFO("Loading plugin from: {}", plugin_path);
  
  // 打开动态库
  void* dl_handle = dlopen(plugin_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
  if (!dl_handle) {
    LOG_ERROR("Failed to load plugin: {}", dlerror());
    return nullptr;
  }
  
  // 获取创建函数
  auto create_func = getCreateFunction(dl_handle);
  if (!create_func) {
    LOG_ERROR("Failed to find createAlgoPlugin function in {}", plugin_path);
    dlclose(dl_handle);
    return nullptr;
  }
  
  // 创建插件实例
  try {
    auto plugin = create_func();
    if (!plugin) {
      LOG_ERROR("createAlgoPlugin returned nullptr");
      dlclose(dl_handle);
      return nullptr;
    }
    
    auto info = plugin->getInfo();
    LOG_INFO("Plugin loaded successfully: {} v{}", info.name, info.version);
    
    // 保存插件信息
    PluginHandle handle;
    handle.dl_handle = dl_handle;
    handle.instance = plugin;
    handle.path = plugin_path;
    loaded_plugins_[info.name] = handle;
    
    return plugin;
  } catch (const std::exception& e) {
    LOG_ERROR("Exception while creating plugin: {}", e.what());
    dlclose(dl_handle);
    return nullptr;
  }
}

inline std::vector<std::string> PluginLoader::scanPlugins(
    const std::string& plugin_dir) {
  std::vector<std::string> plugin_files;
  
  // TODO: 使用 std::filesystem (C++17) 或 dirent.h 扫描目录
  LOG_WARN("scanPlugins not fully implemented yet");
  
  return plugin_files;
}

inline std::vector<std::shared_ptr<AlgoPluginBase>> 
PluginLoader::loadPluginsFromDir(const std::string& plugin_dir) {
  std::vector<std::shared_ptr<AlgoPluginBase>> plugins;
  
  auto plugin_files = scanPlugins(plugin_dir);
  for (const auto& file : plugin_files) {
    auto plugin = loadPlugin(file);
    if (plugin) {
      plugins.push_back(plugin);
    }
  }
  
  return plugins;
}

inline bool PluginLoader::unloadPlugin(const std::string& plugin_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = loaded_plugins_.find(plugin_name);
  if (it == loaded_plugins_.end()) {
    LOG_WARN("Plugin not found: {}", plugin_name);
    return false;
  }
  
  // 先释放插件实例（调用析构函数）
  it->second.instance.reset();
  
  // 然后关闭动态库
  if (it->second.dl_handle) {
    dlclose(it->second.dl_handle);
  }
  
  loaded_plugins_.erase(it);
  LOG_INFO("Plugin unloaded: {}", plugin_name);
  
  return true;
}

inline void PluginLoader::unloadAll() {
  std::lock_guard<std::mutex> lock(mutex_);
  
  for (auto& pair : loaded_plugins_) {
    pair.second.instance.reset();
    if (pair.second.dl_handle) {
      dlclose(pair.second.dl_handle);
    }
  }
  
  loaded_plugins_.clear();
  LOG_INFO("All plugins unloaded");
}

inline std::vector<std::string> PluginLoader::getLoadedPlugins() const {
  std::lock_guard<std::mutex> lock(mutex_);
  
  std::vector<std::string> names;
  for (const auto& pair : loaded_plugins_) {
    names.push_back(pair.first);
  }
  
  return names;
}

inline std::shared_ptr<AlgoPluginBase> PluginLoader::getPlugin(
    const std::string& plugin_name) {
  std::lock_guard<std::mutex> lock(mutex_);
  
  auto it = loaded_plugins_.find(plugin_name);
  if (it != loaded_plugins_.end()) {
    return it->second.instance;
  }
  
  return nullptr;
}

inline bool PluginLoader::isValidPluginFile(const std::string& filename) const {
  // 检查文件扩展名是否为 .so
  return filename.size() > 3 && 
         filename.substr(filename.size() - 3) == ".so";
}

inline CreateAlgoPluginFunc PluginLoader::getCreateFunction(void* dl_handle) {
  // 清除之前的错误
  dlerror();
  
  // 查找 createAlgoPlugin 函数
  void* func_ptr = dlsym(dl_handle, "createAlgoPlugin");
  
  const char* error = dlerror();
  if (error) {
    LOG_ERROR("dlsym error: {}", error);
    return nullptr;
  }
  
  return reinterpret_cast<CreateAlgoPluginFunc>(func_ptr);
}

}  // namespace plugin
}  // namespace infer_frame
