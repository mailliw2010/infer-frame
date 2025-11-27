# REQ-001: Backend 抽象层

## 需求概述

推理框架必须支持多种推理后端（TensorRT, ONNXRuntime, OpenVINO, RKNN 等），并确保算法代码与推理后端完全解耦。

## 背景

### 问题陈述

在旧版 VSE 项目中发现以下问题：
1. 算法代码直接依赖 TensorRT API（如 `trt_engine_`），导致算法与后端强耦合
2. 更换部署平台（如从 Jetson 迁移到 RK3588）需要重写算法代码
3. 团队协作困难：算法工程师需要掌握多种推理后端的 API

### 参考实现

FastDeploy Runtime 提供了良好的 Backend 抽象模式：
- 统一的 `Runtime` 接口
- 配置驱动的后端选择
- 算法代码仅依赖抽象接口

## 功能需求

### FR-001-1: 统一推理接口

**描述**：提供统一的 `BackendInterface` 抽象基类

**接口定义**：
```cpp
class BackendInterface {
 public:
  virtual base::Status init(const BackendConfig& config) = 0;
  virtual base::Status infer(
      const std::vector<device::Tensor*>& inputs,
      std::vector<device::Tensor*>& outputs) = 0;
  virtual base::Status inferBatch(...) = 0;
  virtual std::vector<TensorInfo> getInputInfos() = 0;
  virtual std::vector<TensorInfo> getOutputInfos() = 0;
  virtual base::Status deinit() = 0;
};
```

**验收标准**：
- [ ] 所有后端实现必须继承 `BackendInterface`
- [ ] 算法插件代码不得直接调用特定后端的 API
- [ ] 支持运行时动态切换后端（通过配置文件）

### FR-001-2: 支持的后端列表

| 后端 | 平台 | 优先级 | 状态 |
|------|------|--------|------|
| TensorRT | Jetson, CUDA | P0 | 待实现 |
| ONNXRuntime | 通用 | P0 | 待实现 |
| RKNN | RK3588/RK3576 | P1 | 待实现 |
| OpenVINO | Intel CPU/GPU | P1 | 待实现 |
| PaddleInference | 通用 | P2 | 待实现 |
| MNN | 移动端 | P2 | 待实现 |

### FR-001-3: 配置驱动的后端选择

**JSON 配置示例**：
```json
{
  "backend": {
    "type": "TensorRT",
    "model_path": "/models/yolov8n.engine",
    "device_id": 0,
    "custom_options": {
      "fp16_mode": "true",
      "workspace_size": "1073741824"
    }
  }
}
```

**验收标准**：
- [ ] 修改配置文件即可切换后端，无需重新编译
- [ ] 配置校验：不支持的后端类型应返回明确错误
- [ ] 平台检测：自动检测当前平台可用的后端（如 Jetson 优先 TensorRT）

## 非功能需求

### NFR-001-1: 性能

- 后端抽象层开销 < 1% 总推理时间
- 支持零拷贝（Tensor 在后端间传递无需额外内存拷贝）

### NFR-001-2: 可维护性

- 新增后端支持仅需实现 `BackendInterface`，无需修改算法代码
- 提供后端单元测试框架

### NFR-001-3: 兼容性

- 兼容 NNDeploy 的 Tensor 和 Device 抽象
- 支持 CUDA/CPU/NPU 多种设备类型

## 依赖关系

- **依赖**：NNDeploy 基础框架（Tensor, Device, Memory 抽象）
- **被依赖**：算法插件系统（REQ-002）

## 验收标准

1. **功能完整性**：
   - [ ] 至少实现 TensorRT 和 ONNXRuntime 两个后端
   - [ ] 通过 YOLOv8 插件在两个后端上运行相同推理任务

2. **代码质量**：
   - [ ] 通过静态代码分析（clang-tidy）
   - [ ] 单元测试覆盖率 > 80%

3. **文档完整性**：
   - [ ] 后端开发者指南
   - [ ] API 文档（Doxygen）

## 参考资料

- FastDeploy Runtime 设计：https://github.com/PaddlePaddle/FastDeploy
- NNDeploy 架构：https://nndeploy-zh.readthedocs.io/
- TensorRT API：https://docs.nvidia.com/deeplearning/tensorrt/

## 更新历史

| 日期 | 版本 | 描述 | 作者 |
|------|------|------|------|
| 2025-10-31 | 1.0 | 初始版本 | AI Assistant |
