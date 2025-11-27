# Infer-Frame## 拉取代码

```angular2html

高性能边缘推理框架，支持多平台部署（Jetson, RKNN, CUDA, Sophon）。git clone --recurse-submodules https://git.cisdigital.cn/qtouch/cisdi-framework/t800/sophon-box/infer-demo.git



## 项目概述# 更新代码

git pull --recurse-submodules

Infer-Frame 是一款基于 NNDeploy 的工业级推理引擎，专为钢铁制造等工业场景的缺陷检测设计。

```

### 核心特性



- **Backend 抽象层**：支持 TensorRT, ONNXRuntime, OpenVINO, RKNN 等 13+ 推理后端## 配置环境变量

- **插件化架构**：算法与推理后端解耦，一套代码多平台运行这一步会初始化相关环境变量，如果是交叉编译，需要更改里面的设备信息参数(IP、用户名、密码等)，以便deploy.sh到目标设备

- **编解码集成**：内置 GStreamer/FFmpeg 支持 RTSP/RTMP 视频流处理

- **gRPC 服务**：提供标准化推理服务接口，支持进程隔离部署```angular2html

- **高性能优化**：流水线并行、批量推理、内存池复用cd infer-demo/

source script/env_setup.bash

### 技术栈

# 参数说明

- **推理框架**：NNDeploy v3.0.1

- **编解码**：GStreamer 1.20+ / FFmpeg 4.4+# 环境变量配置

- **通信协议**：gRPC 1.50+# 项目路径

- **构建工具**：CMake 3.18+, C++17export PROJECT_ROOT=$(dirname "$DIR")

# 部署设备IP

## 快速开始export DEVICE_IP=192.168.11.75

# 部署设备用户名

### 环境要求export DEVICE_USERNAME=HwHiAiUser

# 部署设备密码

```bashexport DEVICE_PASSWD=Mind@123

# Ubuntu 20.04/22.04# 部署设备远程调试端口

sudo apt-get install -y \export DEVICE_GDB_SERVER_PORT=123

    build-essential cmake git \# 部署二进制的路径

    libgstreamer1.0-dev \export DEVICE_DEPLOY_DIR=/home/HwHiAiUser/xcd

    libgstreamer-plugins-base1.0-dev \

    libavcodec-dev libavformat-dev \

    protobuf-compiler grpc++```

```## 编译



### 编译构建编译环境一般在服务器，首先进入开发环境，将源码放到服务器,挂载路径统一在/data下

* 算能：192.168.11.51:2022

```bash* 昇腾：192.168.11.53:2032

# 1. 克隆仓库（包含子模块）

git clone --recursive https://github.com/yourorg/infer-frame.git

cd infer-frame### 编译所有算法[TODO]

```angular2html

# 2. 编译（Jetson 平台）# 进入主目录，执行脚本

mkdir build && cd buildcd ${PROJECT_DIR}

cmake -DCMAKE_TOOLCHAIN_FILE=../platforms/aarch64_jetson.cmake ..# debug版本

make -j$(nproc)build.sh

# release版本

# 3. 安装build.sh release

sudo make install```

```

### 编译单个算法

### 运行示例```angular2html

# 进入某个算法目录，执行脚本

```bashcd impl/sophon/yolov5_offduty_infer_fastdeploy

# 启动推理服务# debug版本

./build/infer_frame_server --config=config/engine_config.jsonbuild.sh

# release版本

# 测试 gRPC 接口build.sh release

grpcurl -plaintext -d '{"camera_id": "test", "rtsp_url": "rtsp://..."}' \# 打包到build/package

    localhost:50051 infer.InferenceService/AddCamerabuild.sh install

```

结果保存在impl/sophon/yolov5_offduty_infer_fastdeploy/build/package下

## 项目结构tree -L 1

.

```├── algo_config.json

infer-frame/├── default_config.yaml

├── specs/                 # 规格文档├── libalgo_offduty.so

│   ├── requirements/      # 需求定义├── libalgo_offduty_yolov5s_1688_f32.bmodel

│   ├── tasks/             # 任务分解├── liblogic_process.so

│   └── design/            # 设计文档

├── src/

│   ├── codec/             # 编解码模块```

│   ├── inference/         # 推理引擎封装### 部署到设备

│   └── grpc_service/      # gRPC 服务实现* 如果使用交叉环境下编译，需要部署到板子上；如果直接在板子上编译，则不需要该步骤[不建议]

├── plugins/               # 算法插件* 编译步骤结束后自动执行部署脚本到默认IP(env_setup.sh中)地址的设备，输入设备密码即可。如果需要临时部署其他设备，则独立执行如下指令：

│   ├── yolov8/```angular2html

│   └── paddleocr/deploy.sh [用户名]@[新ip地址]:[部署路径]

├── 3rdparty/```

│   └── nndeploy/          # Git Submodule## 运行测试代码

├── proto/                 # gRPC 协议定义首先执行编译，然后：

├── config/                # 配置文件```angular2html

└── CMakeLists.txt# 进入算法目录

```cd impl/sophon/yolov5_offduty_infer_fastdeploy

# 运行

## 文档bash run.bash [gdb]

# 使用gdb模式，端口号在script/env_setup.sh中指定，如果需要修改，需重新在根目录下source script/env_setup.sh

- [架构设计](specs/design/architecture.md)

- [Backend 抽象层设计](specs/design/backend-design.md)# 根据需求修改default_config.yaml中的图片与推理模型

- [插件开发指南](specs/design/plugin-lifecycle.md)

- [gRPC API 文档](specs/design/grpc-api-design.md)├── build

│   ├── demo    #测试程序

## 协作项目│   ├── libalgo_offduty.so

│   └── liblogic_process.so

- [manage-system](../manage-system): Go 管理平台（云端/边缘）│   └── algo_config.json # 模型配置文件

- [vse](../vse): 旧版 VSE 项目（参考实现）│   └── xxx.bmodel  # 模型名字

├── demo

## 许可证│   ├── data

│   │   ├── image #测试图

Apache 2.0│   │   │   ├── vlcsnap-2024-10-15-15h03m04s125.jpg

│   │   │   └── yolov5-test.jpg
│   │   └── model  #模型
│   │       └── yolov5s_v6.1_3output_int8_1b_2core.bmodel
│   ├── main.cpp
│   └── out #输出
├── run.bash  #运行脚本



```


## 说明

```angular2html
.
├── 3rdparty                        第三方库
│   ├── nlohmann-json               json解析
│   └── spdlog                      日志
│   └── FastDeploy                  第三方推理框架，可选
├── CMakeLists.txt                  CMake构建
├── cmake                           CMake构建脚本
├── README.md                       说明
├── common                          公共接口(只放公共使用的依赖)
│   └── infer_process               视觉产品部接口
│   └── utils                       公共utils
├── impl                            ****主要的算法实现，包括测试demo****
│   └── sophon                      算能平台的算法实现
│   └── Ascend                      昇腾平台的算法实现
├── platform                        硬件平台提供的库与依赖
│   ├── ascend                      昇腾平台（昇腾独立的依赖）
│   └── sophon                      算能平台（算能独立的依赖）
└── script                          运行脚本
    └── build.sh                    


```


# 粗略规划
1. 重构金眼SDK，分离公共代码和业务代码 [已完成]
2. 接入FastDeploy框架，算法部署统一管理，支持算能/昇腾/GPU/CPU等异构平台的算法部署；[进行中]
    * 算能；[已适配]
    * 昇腾；
    * CPU；
    * GPU；
3. 编写构建脚本,支持服务器编译、交叉编译、边缘编译；[已完成]
```angular2html
build.sh [debug][release][clean]
build.sh 3rdparty [debug][release][clean]
```
4. 编写运行脚本；[已完成]
```angular2html
bash run.sh [gdb]
```
5. 新增日志单元； 
6. 新增统一配置单元；

形如
```angular2html
platform: 
  name: sophon
algo:
  input: demo/data/image
  model_dir: /xxx
  confidence: 0.5
  nms: 0.5
  multi_label: false
  osd: true
media:
  encode: true
  # stream url|file directory
  target: 192.168.11.73:8554/rtsp/1
  
```
```
设计基本架构
1. 配置优先级机制
   配置值的来源按优先级覆盖：
   环境变量 的优先级最高，可以动态注入和覆盖其他值。
   YAML 配置文件 次之，作为主要的静态配置来源。
   默认配置 最低，兜底使用，避免缺失配置。
2. 配置加载流程
   加载默认配置。
   加载 YAML 配置文件（如果存在），覆盖默认配置。
   加载环境变量，覆盖 YAML 配置和默认配置。
3. 配置访问接口
   提供统一的接口（如 get(key) 或 config['key']）访问配置值，内部自动解析优先级。
```

7. 新增微服务；
8. 容器化构建；