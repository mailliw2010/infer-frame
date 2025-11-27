# REQ-002: 算法插件系统

## 需求概述

提供标准化的算法插件接口，支持动态加载 .so 文件，实现算法与推理框架的解耦。

## 背景

### 问题陈述

旧版 VSE 项目中算法集成存在以下痛点：
1. 算法库编译进主程序，更新算法需重新编译整个框架
2. 多团队协作困难：算法开发者必须理解框架内部实现
3. 无法热更新：生产环境更新算法需停机重启

### 目标

实现类似 NNDeploy Plugin 的机制：
- 算法以独立 .so 文件形式存在
- 运行时动态加载/卸载
- 标准化接口，算法开发者只需关注前后处理逻辑

## 功能需求

### FR-002-1: 插件接口定义

**基础接口**：
```cpp
class AlgoPluginBase {
 public:
  // 获取插件信息
  virtual AlgoInfo getInfo() = 0;
  
  // 初始化（加载模型、配置后端）
  virtual base::Status init(
      const std::string& model_path,
      const BackendConfig& backend_config,
      const std::map<std::string, std::string>& algo_params) = 0;
  
  // 单帧推理
  virtual base::Status infer(
      std::vector<device::Tensor*>& inputs,
      std::vector<device::Tensor*>& outputs) = 0;
  
  // 批量推理
  virtual base::Status inferBatch(
      const std::vector<std::vector<device::Tensor*>>& batch_inputs,
      std::vector<std::vector<device::Tensor*>>& batch_outputs) = 0;
  
  // 资源释放
  virtual base::Status deinit() = 0;
};
```

**插件元信息**：
```cpp
struct AlgoInfo {
  std::string name;                      // "YOLOv8"
  std::string version;                   // "1.0.0"
  AlgoType type;                         // kDetection
  std::string description;
  std::vector<BackendType> supported_backends;
};
```

**验收标准**：
- [ ] 所有算法插件必须继承 `AlgoPluginBase`
- [ ] 插件不得依赖框架内部实现（仅依赖公共接口）
- [ ] 支持查询插件支持的后端列表

### FR-002-2: 插件注册机制

**宏注册**：
```cpp
// 插件实现文件
class YOLOv8Plugin : public AlgoPluginBase {
  // ... 实现
};

NNDEPLOY_REGISTER_ALGO_PLUGIN(YOLOv8Plugin);
```

**动态加载**：
```cpp
// 框架侧
class PluginLoader {
 public:
  std::shared_ptr<AlgoPluginBase> loadPlugin(
      const std::string& plugin_path);  // "/path/to/libyolov8_plugin.so"
  
  std::vector<std::string> scanPlugins(
      const std::string& plugin_dir);   // "/usr/local/lib/infer-frame/plugins/"
};
```

**验收标准**：
- [ ] 支持 `dlopen` 动态加载 .so 文件
- [ ] 插件加载失败不影响主程序稳定性
- [ ] 支持插件目录扫描（自动发现可用插件）

### FR-002-3: 插件生命周期管理

**状态机**：
```
┌─────────┐  loadPlugin()   ┌────────┐  init()    ┌──────────┐
│ Unloaded├────────────────→│ Loaded ├───────────→│ Initialized│
└─────────┘                 └────────┘            └─────┬──────┘
                                                         │
                                                  infer()│
                                                         │
                                                    ┌────▼────┐
                                            deinit()│ Running │
                                            ┌───────┤         │
                                            │       └─────────┘
                                            ▼
                                      ┌──────────┐
                                      │ Stopped  │
                                      └──────────┘
```

**验收标准**：
- [ ] 支持插件热重载（deinit → unload → load → init）
- [ ] 异常插件自动隔离（不影响其他插件）
- [ ] 提供插件健康检查接口

### FR-002-4: 内置插件示例

| 插件名 | 类型 | 后端支持 | 状态 |
|--------|------|----------|------|
| YOLOv8Plugin | 目标检测 | TensorRT, ONNX, RKNN | 待实现 |
| PaddleOCRPlugin | OCR | PaddleInference, ONNX | 待实现 |
| SegmentPlugin | 图像分割 | TensorRT, ONNX | 待实现 |

## 非功能需求

### NFR-002-1: 性能

- 插件加载时间 < 100ms（不含模型加载）
- 插件接口调用开销 < 10μs（虚函数调用）

### NFR-002-2: 兼容性

- 支持 C++17 标准编译的插件
- ABI 兼容性：使用 C 接口导出符号

### NFR-002-3: 安全性

- 插件沙箱：恶意插件不得访问框架内存
- 符号隔离：使用 `RTLD_LOCAL` 避免符号冲突

## 开发工作流

### 算法开发者视角

```bash
# 1. 创建插件项目
cd plugins/my_algo/
cat > my_algo_plugin.cpp << 'EOF'
#include "algo_plugin_base.h"

class MyAlgoPlugin : public AlgoPluginBase {
  // 实现 getInfo, init, infer, inferBatch, deinit
};

NNDEPLOY_REGISTER_ALGO_PLUGIN(MyAlgoPlugin);
EOF

# 2. 编译插件
mkdir build && cd build
cmake ..
make

# 3. 安装插件
sudo cp libmy_algo_plugin.so /usr/local/lib/infer-frame/plugins/
```

### 框架使用者视角

```json
{
  "workflows": [{
    "nodes": [{
      "id": "my_algo",
      "type": "algo_plugin",
      "config": {
        "plugin_name": "MyAlgo",
        "plugin_version": "1.0.0",
        "backend": {"type": "TensorRT", ...}
      }
    }]
  }]
}
```

## 依赖关系

- **依赖**：Backend 抽象层（REQ-001）
- **被依赖**：gRPC 服务接口（REQ-003）

## 验收标准

1. **功能测试**：
   - [ ] 成功加载并运行 YOLOv8Plugin
   - [ ] 插件热重载不导致内存泄漏
   - [ ] 损坏的 .so 文件不导致主程序崩溃

2. **性能测试**：
   - [ ] 1000 次插件加载/卸载，耗时 < 100s
   - [ ] 插件推理性能与静态链接版本差异 < 5%

3. **文档**：
   - [ ] 插件开发者指南（含完整示例）
   - [ ] 插件 API 参考文档

## 参考资料

- NNDeploy Plugin 系统：https://github.com/nndeploy/nndeploy/tree/main/plugin
- dlopen 手册：https://man7.org/linux/man-pages/man3/dlopen.3.html

## 更新历史

| 日期 | 版本 | 描述 | 作者 |
|------|------|------|------|
| 2025-10-31 | 1.0 | 初始版本 | AI Assistant |
