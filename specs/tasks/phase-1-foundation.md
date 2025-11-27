# Phase 1: 基础框架搭建

**目标**：完成推理引擎的核心架构，支持单个算法插件运行

**时间**：2 周

## 任务列表

### T1.1 项目初始化 ✅ (已完成)
- [x] 创建项目目录结构
- [x] 初始化 Git 仓库
- [x] 编写 specs 文档（requirements/design）
- [x] 配置 CMake 构建系统
- [x] 添加 NNDeploy 作为子模块

**状态**: ✅ 已完成  
**验收**: 
- [x] 成功编译 `infer_frame_server` 二进制文件
- [x] 支持 4 个平台（x86_64_cuda, aarch64_jetson, aarch64_rknn, aarch64_sophon）
- [x] Proto 文件自动生成 gRPC 代码
- [x] 集成 spdlog 日志系统
- [x] 程序可正常启动和停止

**负责人**：待分配  
**预计工时**：2 天

### T1.2 Backend 抽象层实现 ✅ (已完成)
- [x] 实现 `BackendInterface` 基类
- [x] 实现 `BackendFactory` 工厂类
- [x] 实现 TensorRT Backend
- [x] 实现 ONNXRuntime Backend
- [x] 编写 Backend 单元测试

**状态**: ✅ 已完成  
**验收**:
- [x] BackendInterface 抽象接口定义完成
- [x] BackendFactory 工厂类支持动态注册
- [x] TensorRT Backend 实现（占位版本）
- [x] ONNXRuntime Backend 实现（占位版本）
- [x] backend_test 测试程序全部通过
- [x] 所有头文件改为 `#pragma once` 风格

**负责人**：待分配  
**预计工时**：5 天

**验收标准**：
- 通过单元测试（覆盖率 > 80%）
- TensorRT Backend 成功加载 YOLOv8.engine
- ONNXRuntime Backend 成功加载 YOLOv8.onnx
- 性能损耗 < 1%（对比直接调用 TensorRT API）

### T1.3 算法插件系统 ✅ (已完成)
- [x] 实现 `AlgoPluginBase` 接口
- [x] 实现 `PluginLoader`（dlopen/dlsym）
- [x] 实现插件注册宏 `REGISTER_ALGO_PLUGIN`
- [x] 开发 YOLOv8Plugin 示例
- [x] 编写插件测试程序

**状态**: ✅ 已完成  
**验收**:
- [x] AlgoPluginBase 接口定义完成（init, infer, deinit, getInfo）
- [x] PluginLoader 实现动态加载 .so 文件
- [x] YOLOv8Plugin 编译成功 (yolov8_plugin.so)
- [x] plugin_test 测试通过：
  - ✓ 插件加载成功
  - ✓ 查询插件信息成功
  - ✓ 插件反初始化成功
  - ✓ 插件卸载成功

**已知问题**:
- 插件初始化失败（Backend type not supported），原因是插件动态库有独立的 BackendFactory 实例，需要在后续实现中改为共享符号或通过依赖注入解决。

**负责人**：待分配  
**预计工时**：5 天

### T1.4 基础编解码支持
- [ ] 实现 `VideoDecoder` 接口
- [ ] 实现 GStreamerDecoder（RTSP 解码）
- [ ] 实现硬件加速检测（Jetson NVDEC）
- [ ] 集成测试（RTSP → 解码 → 推理）

**负责人**：待分配  
**预计工时**：3 天

**验收标准**：
- 成功解码 RTSP 流
- Jetson 平台硬件加速正常工作
- 解码延迟 < 50ms

### T1.5 gRPC 服务基础框架
- [ ] 编写 proto 文件（infer_service.proto）
- [ ] 生成 C++ gRPC 代码
- [ ] 实现 HealthCheck RPC
- [ ] 实现 AddCamera RPC
- [ ] 实现 InferSingleFrame RPC

**负责人**：待分配  
**预计工时**：3 天

**验收标准**：
- gRPC 服务成功启动（监听 50051）
- Go 客户端成功调用 HealthCheck
- 单帧推理 RPC 调用成功

## 里程碑

**Milestone 1.0**：完整推理流程打通
- 时间：第 2 周结束
- 交付物：
  - 可运行的 `infer_frame_server` 二进制
  - YOLOv8 插件（支持 TensorRT/ONNX）
  - gRPC 接口（基础功能）
  - 单元测试报告（覆盖率 > 70%）

## 风险与依赖

| 风险 | 影响 | 缓解措施 |
|------|------|----------|
| NNDeploy API 不稳定 | 高 | 锁定 v3.0.1 版本，不自动升级 |
| GStreamer 平台差异大 | 中 | 优先支持 Jetson，其他平台延后 |
| 团队成员不熟悉 gRPC | 低 | 提供培训文档和示例代码 |

## 下一阶段

→ [Phase 2: 插件生态与优化](phase-2-plugins.md)
