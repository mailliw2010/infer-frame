# REQ-004: 编解码模块

## 需求概述

提供统一的视频编解码接口，支持 RTSP/RTMP/本地文件解码，以及 H.264/H.265 编码输出。

## 背景

### 问题陈述

工业场景的视频来源多样：
1. **RTSP 摄像头**：钢厂现场 IP Camera（主流）
2. **RTMP 推流**：移动端实时推流
3. **本地视频文件**：历史录像分析
4. **USB 摄像头**：测试环境

### 技术选型

| 方案 | 优势 | 劣势 | 选择 |
|------|------|------|------|
| GStreamer | 硬件加速、插件丰富 | API 复杂 | ✅ 主要方案 |
| FFmpeg | API 简单、兼容性好 | 硬件加速配置繁琐 | ✅ 备用方案 |
| OpenCV | 简单易用 | 性能差、无硬件加速 | ❌ 仅测试用 |

## 功能需求

### FR-004-1: 视频解码接口

**统一抽象**：
```cpp
class VideoDecoder {
 public:
  virtual base::Status init(const DecodeConfig& config) = 0;
  
  // 获取一帧（阻塞）
  virtual base::Status grabFrame(cv::Mat& frame, int64_t& timestamp) = 0;
  
  // 获取解码信息
  virtual VideoInfo getVideoInfo() = 0;
  
  virtual base::Status deinit() = 0;
};

struct DecodeConfig {
  std::string source_url;      // "rtsp://192.168.1.100/stream"
  DecodeBackend backend;       // kGStreamer, kFFmpeg
  bool enable_hw_accel;        // Jetson NVDEC, RKNN RKVDEC
  int queue_size;              // 解码缓冲队列大小
};

struct VideoInfo {
  int width;
  int height;
  int fps;
  std::string codec;           // "H264", "H265"
};
```

**验收标准**：
- [ ] 支持 RTSP/RTMP/HTTP/本地文件
- [ ] 硬件加速解码（Jetson NVDEC 利用率 > 80%）
- [ ] 断流自动重连（最多重试 10 次）

### FR-004-2: GStreamer 实现

**Pipeline 示例**：
```cpp
class GStreamerDecoder : public VideoDecoder {
 private:
  GstElement* pipeline_;
  GstElement* source_;
  GstElement* decoder_;
  GstElement* converter_;
  GstElement* sink_;
  
  std::string buildPipelineStr(const DecodeConfig& config) {
    if (config.enable_hw_accel && isJetson()) {
      // Jetson 硬件加速
      return fmt::format(
          "rtspsrc location={} latency=0 ! "
          "rtph264depay ! h264parse ! "
          "nvv4l2decoder ! nvvidconv ! "
          "video/x-raw,format=BGRx ! appsink",
          config.source_url);
    } else {
      // CPU 解码
      return fmt::format(
          "rtspsrc location={} ! "
          "rtph264depay ! avdec_h264 ! "
          "videoconvert ! appsink",
          config.source_url);
    }
  }
};
```

**平台适配**：
| 平台 | 解码器 | 验证 |
|------|--------|------|
| Jetson | nvv4l2decoder | 待测试 |
| RK3588 | mppvideodec | 待测试 |
| x86 CUDA | nvdec | 待测试 |
| CPU | avdec_h264 | 待测试 |

### FR-004-3: 视频编码接口

**编码器抽象**：
```cpp
class VideoEncoder {
 public:
  virtual base::Status init(const EncodeConfig& config) = 0;
  
  // 编码一帧
  virtual base::Status encodeFrame(
      const cv::Mat& frame, 
      std::vector<uint8_t>& encoded_data) = 0;
  
  virtual base::Status deinit() = 0;
};

struct EncodeConfig {
  int width;
  int height;
  int fps;
  int bitrate;               // kbps
  std::string codec;         // "H264", "H265"
  bool enable_hw_accel;
};
```

**应用场景**：
1. **渲染推流**：检测结果绘制后推送 RTMP
2. **告警录像**：异常帧保存为视频文件
3. **缩略图生成**：JPEG 编码上传云端

**验收标准**：
- [ ] 支持 H.264/H.265 编码
- [ ] 编码延迟 < 20ms（1080p@30fps，硬件加速）
- [ ] 输出码率稳定（误差 < 10%）

### FR-004-4: 性能优化

**零拷贝路径**：
```
RTSP Stream → NVDEC (GPU Memory) → NNDeploy Tensor (GPU) → Inference
            ↓ (仅在需要时拷贝到 CPU)
          cv::Mat (CPU Memory)
```

**批量解码**：
- 多路摄像头共享解码线程池
- 解码队列动态调整（避免内存爆炸）

**验收标准**：
- [ ] 单台 Jetson NX 支持 16 路 1080p@25fps 解码
- [ ] GPU-CPU 内存拷贝次数 < 2 次/帧

## 非功能需求

### NFR-004-1: 稳定性

- RTSP 断流恢复时间 < 5s
- 连续运行 7 天无内存泄漏

### NFR-004-2: 兼容性

- 支持 GStreamer 1.14+（Ubuntu 18.04）
- 支持 FFmpeg 4.x/5.x/6.x

## 依赖关系

- **依赖**：系统依赖（GStreamer/FFmpeg）
- **被依赖**：gRPC 服务接口（REQ-003）

## 验收标准

1. **功能测试**：
   - [ ] 成功解码 RTSP/RTMP/本地文件
   - [ ] 硬件加速正常工作（Jetson/RK3588）

2. **性能测试**：
   - [ ] 16 路 1080p 解码，CPU 占用 < 50%
   - [ ] GPU 解码利用率 > 80%

3. **文档**：
   - [ ] GStreamer Pipeline 调试指南
   - [ ] 平台适配文档

## 参考资料

- GStreamer 文档：https://gstreamer.freedesktop.org/documentation/
- Jetson 多媒体 API：https://docs.nvidia.com/jetson/l4t-multimedia/

## 更新历史

| 日期 | 版本 | 描述 | 作者 |
|------|------|------|------|
| 2025-10-31 | 1.0 | 初始版本 | AI Assistant |
