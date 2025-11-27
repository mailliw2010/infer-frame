#pragma once

#include "plugin/algo_plugin_base.h"
#include <memory>

/**
 * @file plugin_registry.h
 * @brief 插件注册宏定义
 * 
 * 每个算法插件必须使用 REGISTER_ALGO_PLUGIN 宏来注册。
 * 该宏会生成导出函数 createAlgoPlugin，供 PluginLoader 调用。
 */

namespace infer_frame {
namespace plugin {

/**
 * @brief 插件注册宏
 * 
 * 在插件实现文件中使用此宏注册插件类。
 * 
 * 使用示例:
 * @code
 * // yolov8_plugin.cc
 * #include "yolov8_plugin.h"
 * #include "plugin/plugin_registry.h"
 * 
 * REGISTER_ALGO_PLUGIN(YOLOv8Plugin)
 * @endcode
 * 
 * @param plugin_class 插件类名
 */
#define REGISTER_ALGO_PLUGIN(plugin_class) \
  extern "C" { \
    std::shared_ptr<infer_frame::plugin::AlgoPluginBase> createAlgoPlugin() { \
      return std::make_shared<plugin_class>(); \
    } \
  }

/**
 * @brief 带版本信息的插件注册宏
 * 
 * 除了注册插件外，还导出版本信息函数。
 * 
 * @param plugin_class 插件类名
 * @param major_ver 主版本号
 * @param minor_ver 次版本号
 * @param patch_ver 补丁版本号
 */
#define REGISTER_ALGO_PLUGIN_WITH_VERSION(plugin_class, major_ver, minor_ver, patch_ver) \
  extern "C" { \
    std::shared_ptr<infer_frame::plugin::AlgoPluginBase> createAlgoPlugin() { \
      return std::make_shared<plugin_class>(); \
    } \
    \
    int getPluginApiVersion() { \
      return (major_ver * 10000 + minor_ver * 100 + patch_ver); \
    } \
  }

}  // namespace plugin
}  // namespace infer_frame
