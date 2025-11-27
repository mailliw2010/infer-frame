#include "plugin/plugin_loader_c.h"
#include "plugin/algo_plugin_interface.h"
#include "utils/one_logger.hpp"

#include <iostream>
#include <cstring>

using namespace infer_frame;

void printTestResult(const std::string& test_name, bool passed) {
  if (passed) {
    LOG_INFO("✓ {}", test_name);
  } else {
    LOG_ERROR("✗ {}", test_name);
  }
}

int main(int argc, char** argv) {
  LOG_INFO("======================================");
  LOG_INFO("  Plugin System Test (C Interface)");
  LOG_INFO("======================================");
  
  // 测试 1: 加载插件
  LOG_INFO("\n[Test 1] Loading YOLOv8 plugin...");
  std::string plugin_path = "./algorithm/yolov8_plugin.so";
  if (argc > 1) {
    plugin_path = argv[1];
  }
  
  plugin::PluginLoaderC loader;
  bool load_success = loader.loadPlugin(plugin_path);
  printTestResult("Load plugin", load_success);
  
  if (!load_success) {
    return 1;
  }
  
  // 测试 2: 查询插件信息
  LOG_INFO("\n[Test 2] Query plugin info...");
  const AlgoInfo* info = loader.getPluginInfo("YOLOv8");
  if (info) {
    LOG_INFO("Plugin name: {}", info->name);
    LOG_INFO("Plugin version: {}", info->version);
    LOG_INFO("Plugin type: {}", static_cast<int>(info->type));
    LOG_INFO("Description: {}", info->description);
    LOG_INFO("Author: {}", info->author);
    LOG_INFO("Supported backends: {}", info->num_backends);
    for (int i = 0; i < info->num_backends; ++i) {
      LOG_INFO("  Backend {}: {}", i, static_cast<int>(info->supported_backends[i]));
    }
    printTestResult("Query plugin info", true);
  } else {
    printTestResult("Query plugin info", false);
    return 1;
  }
  
  // 测试 3: 创建算法实例
  LOG_INFO("\n[Test 3] Creating algorithm instance...");
  AlgoHandle handle = loader.createAlgoInstance("YOLOv8");
  printTestResult("Create instance", handle != nullptr);
  
  if (!handle) {
    return 1;
  }
  
  // 测试 4: 初始化算法（使用 TensorRT Backend）
  LOG_INFO("\n[Test 4] Initializing with TensorRT backend...");
  AlgoInitParam init_param;
  init_param.model_path = "/path/to/yolov8.engine";
  init_param.backend = ALGO_BACKEND_TENSORRT;
  init_param.device_id = 0;
  init_param.config_json = R"({
    "conf_threshold": 0.25,
    "nms_threshold": 0.45,
    "input_width": 640,
    "input_height": 640
  })";
  
  AlgoStatus status = loader.initAlgo(handle, "YOLOv8", &init_param);
  printTestResult("Initialize (TensorRT)", status == ALGO_STATUS_SUCCESS);
  
  if (status != ALGO_STATUS_SUCCESS) {
    LOG_ERROR("Init failed with status: {}", static_cast<int>(status));
  }
  
  // 测试 5: 执行推理
  LOG_INFO("\n[Test 5] Running inference...");
  
  // 创建输入 Tensor
  AlgoTensor input;
  strcpy(input.name, "images");
  input.data_type = ALGO_DATA_TYPE_FLOAT32;
  input.ndim = 4;
  input.shape[0] = 1;    // batch
  input.shape[1] = 3;    // channels
  input.shape[2] = 640;  // height
  input.shape[3] = 640;  // width
  input.size = 1 * 3 * 640 * 640 * sizeof(float);
  std::vector<float> dummy_data(input.size / sizeof(float), 0.5f);
  input.data = dummy_data.data();
  
  // 执行推理
  AlgoDetResult result;
  status = loader.inferDetection(handle, "YOLOv8", &input, &result);
  printTestResult("Inference", status == ALGO_STATUS_SUCCESS);
  
  if (status == ALGO_STATUS_SUCCESS) {
    LOG_INFO("Detected {} objects:", result.num_boxes);
    for (int i = 0; i < result.num_boxes; ++i) {
      const AlgoDetBox& box = result.boxes[i];
      LOG_INFO("  [{}] {} - score: {:.2f}, bbox: ({:.1f}, {:.1f}, {:.1f}, {:.1f})",
               i, box.class_name, box.score, box.x1, box.y1, box.x2, box.y2);
    }
  }
  
  // 测试 6: 反初始化
  LOG_INFO("\n[Test 6] Deinitializing...");
  status = loader.deinitAlgo(handle, "YOLOv8");
  printTestResult("Deinitialize", status == ALGO_STATUS_SUCCESS);
  
  // 测试 7: 销毁实例
  LOG_INFO("\n[Test 7] Destroying instance...");
  loader.destroyAlgoInstance(handle, "YOLOv8");
  printTestResult("Destroy instance", true);
  
  // 测试 8: 卸载插件
  LOG_INFO("\n[Test 8] Unloading plugin...");
  bool unload_success = loader.unloadPlugin("YOLOv8");
  printTestResult("Unload plugin", unload_success);
  
  LOG_INFO("\n======================================");
  LOG_INFO("  All tests completed!");
  LOG_INFO("======================================");
  
  return 0;
}
