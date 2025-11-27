#include "plugin/plugin_loader.h"
#include "plugin/algo_plugin_base.h"
#include "inference/backend_factory.h"
#include "inference/base/types.h"
#include "utils/one_logger.hpp"

#include <iostream>
#include <memory>

using namespace infer_frame;

// 声明后端初始化函数
namespace infer_frame {
namespace backend {
extern void initializeBackends();
}
}

void printTestResult(const std::string& test_name, bool passed) {
  if (passed) {
    LOG_INFO("✓ {}", test_name);
  } else {
    LOG_ERROR("✗ {}", test_name);
  }
}

int main(int argc, char** argv) {
  LOG_INFO("======================================");
  LOG_INFO("  Plugin System Test");
  LOG_INFO("======================================");
  
  // 初始化后端工厂
  backend::initializeBackends();
  
  // 测试 1: 加载插件库并获取实例
  LOG_INFO("\n[Test 1] Loading YOLOv8 plugin library...");
  std::string plugin_path = "./libyolov8_plugin.so";
  if (argc > 1) {
    plugin_path = argv[1];
  }
  
  plugin::PluginLoader loader;
  auto plugin = loader.loadPlugin(plugin_path);
  if (!plugin) {
    LOG_ERROR("Failed to load plugin from: {}", plugin_path);
    printTestResult("Load plugin library", false);
    return 1;
  }
  printTestResult("Load plugin library", true);
  
  // 测试 2: 查询插件信息
  LOG_INFO("\n[Test 2] Query plugin info...");
  auto info = plugin->getInfo();
  LOG_INFO("Plugin name: {}", info.name);
  LOG_INFO("Plugin version: {}", info.version);
  LOG_INFO("Plugin type: {}", static_cast<int>(info.type));
  LOG_INFO("Description: {}", info.description);
  LOG_INFO("Supported backends: {}", info.supported_backends.size());
  printTestResult("Query plugin info", info.name == "YOLOv8");
  
  // 测试 3: 初始化插件
  LOG_INFO("\n[Test 3] Initializing plugin...");
  backend::BackendConfig config;
  config.model_path = "/path/to/yolov8.engine";
  config.device_id = 0;
  
  std::map<std::string, std::string> params;
  params["conf_threshold"] = "0.25";
  params["nms_threshold"] = "0.45";
  params["input_width"] = "640";
  params["input_height"] = "640";
  params["backend_type"] = std::to_string(static_cast<int>(backend::BackendType::kTensorRT));
  
  auto init_status = plugin->init(config.model_path, config, params);
  if (!init_status.ok()) {
    LOG_ERROR("Failed to initialize plugin: {}", init_status.message());
    printTestResult("Initialize plugin", false);
  } else {
    printTestResult("Initialize plugin", true);
    LOG_INFO("Initialized: {}", plugin->isInitialized());
  }
  
  // 测试 4: 反初始化
  LOG_INFO("\n[Test 4] Deinitializing plugin...");
  auto deinit_status = plugin->deinit();
  printTestResult("Deinitialize plugin", deinit_status.ok());
  LOG_INFO("Initialized: {}", plugin->isInitialized());
  
  // 测试 5: 卸载插件
  LOG_INFO("\n[Test 5] Unloading plugin...");
  bool unload_status = loader.unloadPlugin("YOLOv8");
  printTestResult("Unload plugin", unload_status);
  
  LOG_INFO("\n======================================");
  LOG_INFO("  All tests completed!");
  LOG_INFO("======================================");
  
  return 0;
}
