# Infer-Frame

高性能边缘推理引擎，支持多平台部署（Jetson、RKNN、CUDA、Sophon），基于 NNDeploy 构建，面向工业/智慧城市场景的目标检测与通用视觉任务。

## 概览

Infer-Frame 提供统一的推理接口与插件化架构，屏蔽各类后端差异，支持在不同硬件平台上以一致的方式进行部署与运行。

### 核心特性

- 后端抽象层：支持 TensorRT、ONNX Runtime、OpenVINO、RKNN、Ascend 等 10+ 推理后端
- 插件化架构：算法与后端解耦，一套代码多平台运行
- 编解码集成：内置 GStreamer/FFmpeg，支持 RTSP/RTMP 视频流
- gRPC 服务：提供标准化推理服务接口，支持进程隔离部署
- 高性能优化：流水线并行、批量推理、内存池复用

### 技术栈

- 推理框架：NNDeploy 3.x
- 编解码：GStreamer 1.20+ / FFmpeg 4.4+
- 通信协议：gRPC
- 构建工具：CMake 3.18+，C++17

## 目录结构

关键目录与文件：

- `CMakeLists.txt`：项目构建配置
- `src/`：服务端核心代码
- `algorithm/`：算法插件（如 `yolov8/`、`paddleocr/`）
- `3rdparty/nndeploy/`：NNDeploy 框架与绑定
- `build/`：构建产物与二进制（如 `infer_frame_server`）
- `proto/`：服务端 gRPC 接口定义
- `config/`：默认配置与模板
- `README.md`：项目说明（本文档）

## 快速开始

### 1. 克隆仓库

```bash
git clone --recurse-submodules <repo-url>
cd infer-frame
git pull --recurse-submodules
```

### 2. 构建

```bash
# 依赖：CMake >= 3.18, GCC/G++ >= 9, protobuf, gRPC, GStreamer/FFmpeg（按需）
source script/env_setup.sh
bash build.sh --clean --type Debug

```

构建完成后，主要二进制位于 `build/`）。

### 3. 运行服务

```bash
# 设置动态库搜索路径（按你的环境调整）
bash run.bash


# 启动服务（二选一）
./build/infer_frame_server
# 或
./build/bin/infer_frame_server
```

### 4. 调用推理（示例）

通过 gRPC 客户端或本地 Python 绑定调用。Python 示例：

```python
from nndeploy.dag import GraphRunner

runner = GraphRunner()
graph_json = {"name_": "Detect_YOLO", "key_": "nndeploy.dag.Graph", /* ... */}
result, cost = runner.run(graph_json, "Detect_YOLO", task_id="demo-1")
print(result, cost)
```

## 配置与动态形状

支持动态形状的三组参数：

- `min_shape_`: 最小输入形状（Object，键为输入名，值为长度为4的数组）
- `opt_shape_`: 最优输入形状（Object）
- `max_shape_`: 最大输入形状（Object）

示例：

```json
{
	"min_shape_": {"input_0": [1, 3, 640, 640]},
	"opt_shape_": {"input_0": [1, 3, 960, 960]},
	"max_shape_": {"input_0": [4, 3, 1280, 1280]},
	"is_dynamic_shape_": true
}
```

注意：这些字段是 JSON 对象（Object），不是数组（Array）。对象成员数量请使用 RapidJSON 的 `MemberCount()` 读取。

## 常见问题与排错

### Worker 进程 SIGABRT（exitcode=-6）

症状：Worker 启动约 9-11 秒后崩溃并重启。

根因：反序列化动态形状参数时使用了 RapidJSON `.Size()` 读取对象成员数量，触发断言 `IsArray()` 导致 `abort()`。

修复：改用 `.MemberCount()` 读取对象成员数量。详情见 `3rdparty/nndeploy/ISSUE_RAPIDJSON_SIZE_BUG.md`。

### 无法生成 core dump

确保：
- `kernel.core_pattern` 设置为可写目录（如 `/tmp/core.%e.%p.%t`）
- 对运行中的进程使用 `prlimit --pid <PID> --core=unlimited:unlimited`

### gRPC 客户端无法连接

- 检查服务端监听地址与端口
- 检查 `LD_LIBRARY_PATH` 是否包含 NNDeploy 依赖库路径

## 贡献

欢迎通过 Pull Request 提交修复与特性。建议：

- 提前讨论接口与目录设计
- 保持代码风格一致
- 添加必要的单元或集成测试

## 许可证

本项目遵循企业内部许可证或另行约定（如适用）。

