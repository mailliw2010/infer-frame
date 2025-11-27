# NVIDIA Jetson Platform Configuration
# For Jetson Orin / Xavier / Nano platforms with TensorRT support

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(ARCH "aarch64")
set(PLATFORM "aarch64")
set(TARGET_PLATFORM "aarch64_jetson" CACHE STRING "")

# Compiler flags optimized for Jetson
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2 -g -Wall -fopenmp" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -fopenmp -DNDEBUG" CACHE STRING "")

# CUDA 环境变量（让 NNDeploy 自己检测）
set(CUDA_TOOLKIT_ROOT_DIR "/usr/local/cuda" CACHE PATH "CUDA installation path")
set(CMAKE_CUDA_COMPILER "/usr/local/cuda/bin/nvcc" CACHE FILEPATH "CUDA compiler")
add_definitions(-DJETSON_PLATFORM)

# TensorRT (pre-installed on Jetson)
set(TENSORRT_ROOT "/usr" CACHE PATH "TensorRT installation directory")
include_directories(${TENSORRT_ROOT}/include/aarch64-linux-gnu)
link_directories(${TENSORRT_ROOT}/lib/aarch64-linux-gnu)
add_definitions(-DENABLE_TENSORRT)

# OpenCV (usually pre-built with CUDA on Jetson)
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
