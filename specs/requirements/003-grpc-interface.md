# REQ-003: gRPC 服务接口

## 需求概述

推理引擎以独立进程运行，通过 gRPC 提供标准化推理服务，与 Go 管理系统实现进程隔离。

## 背景

### 问题陈述

之前讨论中确定了以下架构决策：
1. **CGO 风险规避**：Go 管理系统通过 CGO 调用 C++ 推理引擎可能导致整个系统崩溃
2. **进程隔离**：推理引擎崩溃不应影响管理系统，反之亦然
3. **语言解耦**：Go 和 C++ 技术栈完全分离，便于团队协作

### 解决方案

- 推理引擎作为独立进程，监听 `127.0.0.1:50051`
- 管理系统通过 gRPC 客户端调用推理服务
- 性能权衡：gRPC 延迟 ~200μs vs CGO ~0.5μs（可接受）

## 功能需求

### FR-003-1: 核心 gRPC 服务

**服务定义**（proto/infer_service.proto）：
```protobuf
syntax = "proto3";
package infer;

service InferenceService {
  // 摄像头管理
  rpc AddCamera(AddCameraRequest) returns (AddCameraResponse);
  rpc RemoveCamera(RemoveCameraRequest) returns (RemoveCameraResponse);
  rpc ListCameras(ListCamerasRequest) returns (ListCamerasResponse);
  
  // 工作流管理
  rpc DeployWorkflow(DeployWorkflowRequest) returns (DeployWorkflowResponse);
  rpc UpdateWorkflow(UpdateWorkflowRequest) returns (UpdateWorkflowResponse);
  rpc DeleteWorkflow(DeleteWorkflowRequest) returns (DeleteWorkflowResponse);
  
  // 推理请求
  rpc InferSingleFrame(InferRequest) returns (InferResponse);
  rpc InferBatch(InferBatchRequest) returns (InferBatchResponse);
  
  // 健康检查
  rpc HealthCheck(HealthCheckRequest) returns (HealthCheckResponse);
  
  // 结果流（服务器推送）
  rpc SubscribeResults(SubscribeRequest) returns (stream InferResult);
}

message AddCameraRequest {
  string camera_id = 1;
  string rtsp_url = 2;
  string workflow_id = 3;
  map<string, string> config = 4;  // FPS, ROI, etc.
}

message InferResult {
  string camera_id = 1;
  string workflow_id = 2;
  int64 timestamp = 3;
  ResultType type = 4;  // NORMAL, ALARM
  bytes image_data = 5;  // JPEG 压缩
  string json_data = 6;  // 检测结果
}
```

**验收标准**：
- [ ] 支持并发 50+ 摄像头接入
- [ ] 单次 gRPC 调用延迟 < 500μs（不含推理时间）
- [ ] 支持双向流（客户端推送帧，服务器推送结果）

### FR-003-2: 进程生命周期管理

**启动参数**：
```bash
./infer_frame_server \
  --grpc_port=50051 \
  --config=/etc/infer-frame/engine_config.json \
  --plugin_dir=/usr/local/lib/infer-frame/plugins \
  --log_level=INFO
```

**健康检查机制**：
```cpp
// 服务端实现
Status HealthCheck(ServerContext* context,
                   const HealthCheckRequest* request,
                   HealthCheckResponse* response) {
  response->set_status(HealthStatus::SERVING);
  response->set_timestamp(getCurrentTimestamp());
  
  // 检查关键资源
  response->set_gpu_available(checkGPUStatus());
  response->set_plugin_count(getLoadedPluginCount());
  
  return Status::OK;
}
```

**验收标准**：
- [ ] 进程启动时间 < 5s（不含模型加载）
- [ ] 支持优雅关闭（接收 SIGTERM 后完成当前推理）
- [ ] 健康检查响应时间 < 10ms

### FR-003-3: 错误处理与恢复

**错误码定义**：
```cpp
enum class InferErrorCode {
  OK = 0,
  CAMERA_NOT_FOUND = 1001,
  WORKFLOW_NOT_FOUND = 1002,
  PLUGIN_LOAD_FAILED = 2001,
  BACKEND_INIT_FAILED = 2002,
  INFERENCE_TIMEOUT = 3001,
  GPU_OUT_OF_MEMORY = 3002,
  INTERNAL_ERROR = 9999,
};
```

**重试策略**：
- 推理失败：最多重试 3 次，指数退避（100ms, 200ms, 400ms）
- GPU OOM：自动降低批量大小
- 摄像头断流：每 30s 重连一次

**验收标准**：
- [ ] 所有 gRPC 错误携带详细错误信息
- [ ] 临时故障（网络抖动）自动恢复
- [ ] 永久故障（插件损坏）返回明确错误

### FR-003-4: 性能监控接口

**Metrics 服务**：
```protobuf
rpc GetMetrics(MetricsRequest) returns (MetricsResponse);

message MetricsResponse {
  int32 active_cameras = 1;
  int32 total_infer_requests = 2;
  double avg_infer_latency_ms = 3;
  double gpu_utilization = 4;
  int64 memory_usage_bytes = 5;
}
```

**验收标准**：
- [ ] 实时统计推理 QPS、延迟、成功率
- [ ] 支持 Prometheus 格式导出

## 非功能需求

### NFR-003-1: 性能

- gRPC 服务吞吐量：>= 10000 RPS（小请求）
- 内存占用：< 100MB（不含模型）
- GPU 利用率：> 90%（推理密集场景）

### NFR-003-2: 可靠性

- 服务可用性：>= 99.9%（允许每天 86s 不可用）
- 崩溃恢复时间：< 10s（管理系统自动重启）

### NFR-003-3: 安全性

- 仅监听 localhost（127.0.0.1），不对外暴露
- 支持 mTLS（可选，用于远程部署）

## 部署架构

```
┌─────────────────────────────────────┐
│  Manage-System (Go Process)         │
│  ┌─────────────────────────────┐    │
│  │ gRPC Client (Circuit Breaker)│   │
│  └──────────┬──────────────────┘    │
└─────────────┼───────────────────────┘
              │ gRPC (localhost:50051)
              ▼
┌─────────────────────────────────────┐
│  Infer-Frame (C++ Process)          │
│  ┌─────────────────────────────┐    │
│  │ gRPC Server                 │    │
│  │  ├─ AddCamera()             │    │
│  │  ├─ DeployWorkflow()        │    │
│  │  └─ SubscribeResults()      │    │
│  └──────┬──────────────────────┘    │
│         ▼                            │
│  ┌─────────────┐  ┌─────────────┐   │
│  │  GStreamer  │  │  NNDeploy   │   │
│  │  Decoders   │  │  Plugins    │   │
│  └─────────────┘  └─────────────┘   │
└─────────────────────────────────────┘
```

## 依赖关系

- **依赖**：算法插件系统（REQ-002）、编解码模块（REQ-004）
- **被依赖**：Manage-System 边缘服务

## 验收标准

1. **集成测试**：
   - [ ] Go 客户端成功调用所有 gRPC 接口
   - [ ] 模拟推理引擎崩溃，管理系统自动重启

2. **压力测试**：
   - [ ] 50 路摄像头同时推理，无丢帧
   - [ ] 连续运行 24 小时无内存泄漏

3. **文档**：
   - [ ] gRPC API 参考文档（含示例代码）
   - [ ] 运维手册（启动参数、监控指标）

## 参考资料

- gRPC C++ 指南：https://grpc.io/docs/languages/cpp/
- Protocol Buffers v3：https://protobuf.dev/

## 更新历史

| 日期 | 版本 | 描述 | 作者 |
|------|------|------|------|
| 2025-10-31 | 1.0 | 初始版本 | AI Assistant |
