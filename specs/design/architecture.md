# 系统架构设计

## 1. 总体架构

### 1.1 进程模型

```
┌──────────────────────────────────────────────────────────┐
│  Manage-System (Go Process, Port 8081)                   │
│  - 摄像头管理                                              │
│  - 工作流编排                                              │
│  - 数据上报                                                │
│  - 进程守护（VSEProcessManager）                          │
└────────────────┬─────────────────────────────────────────┘
                 │ gRPC (127.0.0.1:50051)
                 │ - AddCamera
                 │ - DeployWorkflow
                 │ - SubscribeResults
                 ▼
┌──────────────────────────────────────────────────────────┐
│  Infer-Frame (C++ Process)                               │
│  ┌────────────────────────────────────────────────┐      │
│  │  gRPC Server (InferenceServiceImpl)            │      │
│  └─────────┬──────────────────────────────────────┘      │
│            │                                              │
│  ┌─────────▼──────────┐  ┌──────────────────────┐       │
│  │  Camera Manager    │  │  Workflow Manager    │       │
│  │  - RTSP 解码       │  │  - 工作流加载        │       │
│  │  - 帧队列管理      │  │  - DAG 调度          │       │
│  └─────────┬──────────┘  └──────────┬───────────┘       │
│            │                         │                    │
│  ┌─────────▼─────────────────────────▼──────────┐       │
│  │  Inference Engine (NNDeploy)                 │       │
│  │  ┌─────────────┐  ┌─────────────────────┐    │       │
│  │  │ Plugin Mgr  │  │  Backend Factory    │    │       │
│  │  │ - YOLOv8    │  │  - TensorRT         │    │       │
│  │  │ - PaddleOCR │  │  - ONNXRuntime      │    │       │
│  │  └─────────────┘  └─────────────────────┘    │       │
│  └──────────────────────────────────────────────┘       │
│                                                           │
│  ┌────────────────────────────────────────────────┐      │
│  │  Codec Layer                                   │      │
│  │  ┌──────────────┐  ┌──────────────────────┐    │      │
│  │  │  GStreamer   │  │  FFmpeg (Fallback)   │    │      │
│  │  │  - NVDEC     │  │  - Software Decode   │    │      │
│  │  └──────────────┘  └──────────────────────┘    │      │
│  └────────────────────────────────────────────────┘      │
└──────────────────────────────────────────────────────────┘
```

### 1.2 核心设计原则

1. **进程隔离**：
   - 推理引擎独立进程，崩溃不影响管理系统
   - 通过 gRPC 通信，避免 CGO 风险

2. **Backend 解耦**：
   - 算法代码不依赖特定推理后端
   - 配置文件驱动后端选择

3. **插件化**：
   - 算法以 .so 文件形式动态加载
   - 支持热重载，无需重启服务

4. **硬件加速**：
   - 优先使用硬件解码器（NVDEC/RKVDEC）
   - GPU 内存零拷贝路径

## 2. 模块设计

### 2.1 Backend 抽象层

**目标**：算法与推理后端完全解耦

**实现**：
```cpp
// 抽象接口
class BackendInterface {
  virtual Status init(const BackendConfig& config) = 0;
  virtual Status infer(Tensors& inputs, Tensors& outputs) = 0;
};

// 工厂类
class BackendFactory {
  static shared_ptr<BackendInterface> create(BackendType type);
};

// 具体实现
class TensorRTBackend : public BackendInterface { ... };
class ONNXRuntimeBackend : public BackendInterface { ... };
class RKNNBackend : public BackendInterface { ... };
```

**配置示例**：
```json
{
  "backend": {
    "type": "TensorRT",
    "model_path": "/models/yolov8n.engine",
    "device_id": 0,
    "fp16": true
  }
}
```

### 2.2 插件系统

**注册机制**：
```cpp
// 插件实现
class YOLOv8Plugin : public AlgoPluginBase {
 private:
  shared_ptr<BackendInterface> backend_;  // 依赖抽象接口
  
 public:
  Status init(const BackendConfig& backend_cfg) override {
    backend_ = BackendFactory::create(backend_cfg.type);
    backend_->init(backend_cfg);
  }
  
  Status infer(Tensors& inputs, Tensors& outputs) override {
    // 前处理
    preprocess(inputs);
    
    // 推理（通过抽象接口）
    backend_->infer(preprocessed, raw_outputs);
    
    // 后处理
    postprocess(raw_outputs, outputs);
  }
};

NNDEPLOY_REGISTER_ALGO_PLUGIN(YOLOv8Plugin);
```

**动态加载**：
```cpp
// 框架侧
PluginLoader loader;
auto plugin = loader.loadPlugin("/usr/local/lib/infer-frame/plugins/libyolov8.so");
plugin->init(config);
plugin->infer(inputs, outputs);
```

### 2.3 编解码层

**GStreamer Pipeline**：
```cpp
class GStreamerDecoder : public VideoDecoder {
 private:
  string buildPipeline(const DecodeConfig& cfg) {
    if (isJetson() && cfg.enable_hw_accel) {
      return "rtspsrc ! rtph264depay ! nvv4l2decoder ! nvvidconv ! appsink";
    } else if (isRK3588() && cfg.enable_hw_accel) {
      return "rtspsrc ! rtph264depay ! mppvideodec ! videoconvert ! appsink";
    } else {
      return "rtspsrc ! rtph264depay ! avdec_h264 ! videoconvert ! appsink";
    }
  }
};
```

### 2.4 gRPC 服务层

**服务实现**：
```cpp
class InferenceServiceImpl : public InferenceService::Service {
 private:
  CameraManager camera_mgr_;
  WorkflowManager workflow_mgr_;
  PluginManager plugin_mgr_;
  
 public:
  Status AddCamera(ServerContext* ctx, 
                   const AddCameraRequest* req,
                   AddCameraResponse* resp) override {
    // 1. 创建解码器
    auto decoder = make_shared<GStreamerDecoder>();
    decoder->init({.source_url = req->rtsp_url()});
    
    // 2. 加载工作流
    auto workflow = workflow_mgr_.getWorkflow(req->workflow_id());
    
    // 3. 启动推理线程
    camera_mgr_.addCamera(req->camera_id(), decoder, workflow);
    
    return Status::OK;
  }
};
```

## 3. 数据流

### 3.1 推理流程

```
RTSP Stream
   ↓
GStreamer Decoder (NVDEC)
   ↓
GPU Memory (NvBufSurface)
   ↓
NNDeploy Tensor (零拷贝)
   ↓
Backend Inference (TensorRT)
   ↓
Plugin Postprocess
   ↓
gRPC Stream (结果推送)
   ↓
Manage-System (结果处理)
```

### 3.2 内存管理

**GPU 内存池**：
```cpp
class GPUMemoryPool {
  vector<shared_ptr<Tensor>> free_tensors_;
  
  shared_ptr<Tensor> allocate(const TensorDesc& desc) {
    // 优先复用
    for (auto& t : free_tensors_) {
      if (t->desc() == desc) return t;
    }
    // 新分配
    return make_shared<Tensor>(desc, DeviceType::kGPU);
  }
  
  void recycle(shared_ptr<Tensor> tensor) {
    free_tensors_.push_back(tensor);
  }
};
```

## 4. 性能优化

### 4.1 流水线并行

```
Camera 1: Decode → Preprocess → Infer → Postprocess
Camera 2:           Decode → Preprocess → Infer → Postprocess
Camera 3:                      Decode → Preprocess → Infer → Postprocess

时间轴 ──────────────────────────────────────────────────→
```

### 4.2 批量推理

```cpp
class BatchInferenceScheduler {
  void schedule() {
    vector<Tensor> batch_inputs;
    
    // 收集多路摄像头的帧
    for (auto& cam : cameras_) {
      if (auto frame = cam->popFrame()) {
        batch_inputs.push_back(frame);
      }
    }
    
    // 批量推理
    if (batch_inputs.size() >= batch_size_) {
      backend_->inferBatch(batch_inputs, batch_outputs);
    }
  }
};
```

### 4.3 性能指标

| 平台 | 配置 | 性能目标 |
|------|------|----------|
| Jetson Orin NX | YOLOv8n + 16路1080p | 25fps/路 |
| RK3588 | YOLOv8n + 8路1080p | 25fps/路 |
| RTX 3060 | YOLOv8m + 32路1080p | 25fps/路 |

## 5. 错误处理

### 5.1 故障隔离

```cpp
class FaultIsolationManager {
  void handlePluginCrash(const string& plugin_name) {
    // 1. 隔离故障插件
    plugin_mgr_.disablePlugin(plugin_name);
    
    // 2. 通知管理系统
    grpc_server_.sendAlert({
      .type = "PLUGIN_CRASH",
      .plugin = plugin_name
    });
    
    // 3. 尝试重载
    if (retry_count < 3) {
      plugin_mgr_.reloadPlugin(plugin_name);
    }
  }
};
```

### 5.2 降级策略

| 故障类型 | 降级方案 |
|----------|----------|
| GPU OOM | 减少批量大小 |
| 解码失败 | 降低分辨率 |
| 插件崩溃 | 切换备用算法 |

## 6. 部署架构

### 6.1 单机部署

```
┌─────────────────────────┐
│  Edge Device (Jetson)   │
│  ┌───────────────────┐  │
│  │ Manage-System     │  │
│  │ (Port 8081)       │  │
│  └────────┬──────────┘  │
│           │ gRPC        │
│  ┌────────▼──────────┐  │
│  │ Infer-Frame       │  │
│  │ (Port 50051)      │  │
│  └───────────────────┘  │
└─────────────────────────┘
```

### 6.2 分布式部署

```
┌──────────────┐
│ Cloud Server │ ← MQTT/HTTP ← ┌────────────┐
│ (管理平台)    │                │ Edge 1     │
└──────────────┘                │ (Jetson)   │
                                └────────────┘
                                ┌────────────┐
                                │ Edge 2     │
                                │ (RK3588)   │
                                └────────────┘
```

## 7. 技术栈总结

| 层次 | 技术选型 | 版本 |
|------|----------|------|
| 推理框架 | NNDeploy | v3.0.1 |
| 推理后端 | TensorRT/ONNX/RKNN | - |
| 编解码 | GStreamer | 1.20+ |
| 通信协议 | gRPC | 1.50+ |
| 构建工具 | CMake | 3.18+ |
| 编程语言 | C++17 | - |

## 参考文档

- [Backend 抽象设计](backend-design.md)
- [插件生命周期](plugin-lifecycle.md)
- [gRPC API 设计](grpc-api-design.md)
