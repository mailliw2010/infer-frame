# YOLOv8 插件独立编译配置
# 可以独立编译，不依赖主程序

cmake_minimum_required(VERSION 3.10)
project(yolov8_plugin_c VERSION 1.0.0)

# C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译类型
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "========================================")
message(STATUS "  YOLOv8 Plugin Configuration")
message(STATUS "========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# ============================================================================
# 选项：选择 Backend
# ============================================================================
option(ENABLE_TENSORRT "Enable TensorRT backend" ON)
option(ENABLE_ONNXRUNTIME "Enable ONNX Runtime backend" ON)
option(ENABLE_OPENVINO "Enable OpenVINO backend" OFF)

# ============================================================================
# 依赖库
# ============================================================================

# TensorRT
if(ENABLE_TENSORRT)
  find_package(CUDA)
  if(CUDA_FOUND)
    find_path(TENSORRT_INCLUDE_DIR NvInfer.h
      HINTS ${CUDA_TOOLKIT_ROOT_DIR}/include
      /usr/include/aarch64-linux-gnu
      /usr/local/cuda/include
    )
    find_library(TENSORRT_LIBRARY nvinfer
      HINTS ${CUDA_TOOLKIT_ROOT_DIR}/lib64
      /usr/lib/aarch64-linux-gnu
      /usr/local/cuda/lib64
    )
    
    if(TENSORRT_INCLUDE_DIR AND TENSORRT_LIBRARY)
      message(STATUS "TensorRT found")
      message(STATUS "  Include: ${TENSORRT_INCLUDE_DIR}")
      message(STATUS "  Library: ${TENSORRT_LIBRARY}")
      add_compile_definitions(ENABLE_TENSORRT)
      set(HAVE_TENSORRT TRUE)
    else()
      message(WARNING "TensorRT not found, disabling TensorRT backend")
      set(HAVE_TENSORRT FALSE)
    endif()
  else()
    message(WARNING "CUDA not found, disabling TensorRT backend")
    set(HAVE_TENSORRT FALSE)
  endif()
endif()

# ONNX Runtime
if(ENABLE_ONNXRUNTIME)
  find_path(ONNXRUNTIME_INCLUDE_DIR onnxruntime_cxx_api.h
    HINTS /usr/include/onnxruntime
    /usr/local/include/onnxruntime
  )
  find_library(ONNXRUNTIME_LIBRARY onnxruntime
    HINTS /usr/lib
    /usr/local/lib
    /usr/lib/aarch64-linux-gnu
  )
  
  if(ONNXRUNTIME_INCLUDE_DIR AND ONNXRUNTIME_LIBRARY)
    message(STATUS "ONNX Runtime found")
    message(STATUS "  Include: ${ONNXRUNTIME_INCLUDE_DIR}")
    message(STATUS "  Library: ${ONNXRUNTIME_LIBRARY}")
    add_compile_definitions(ENABLE_ONNXRUNTIME)
    set(HAVE_ONNXRUNTIME TRUE)
  else()
    message(WARNING "ONNX Runtime not found, disabling ONNX backend")
    set(HAVE_ONNXRUNTIME FALSE)
  endif()
endif()

# ============================================================================
# 源文件
# ============================================================================

set(SOURCES
  yolov8_plugin_c.cpp
)

# ============================================================================
# 编译为动态库
# ============================================================================

add_library(${PROJECT_NAME} SHARED ${SOURCES})

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# 链接库
set(LINK_LIBS)

if(HAVE_TENSORRT)
  target_include_directories(${PROJECT_NAME} PRIVATE ${TENSORRT_INCLUDE_DIR})
  list(APPEND LINK_LIBS ${TENSORRT_LIBRARY})
endif()

if(HAVE_ONNXRUNTIME)
  target_include_directories(${PROJECT_NAME} PRIVATE ${ONNXRUNTIME_INCLUDE_DIR})
  list(APPEND LINK_LIBS ${ONNXRUNTIME_LIBRARY})
endif()

target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})

# 输出设置
set_target_properties(${PROJECT_NAME} PROPERTIES
  OUTPUT_NAME "yolov8_plugin"
  PREFIX ""
  VERSION ${PROJECT_VERSION}
  SOVERSION 1
)

# ============================================================================
# 安装
# ============================================================================

install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION lib/infer_frame/algorithm
  ARCHIVE DESTINATION lib/infer_frame/algorithm
)

# ============================================================================
# 总结
# ============================================================================

message(STATUS "========================================")
message(STATUS "  Build Summary")
message(STATUS "========================================")
message(STATUS "Plugin Name: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "TensorRT: ${HAVE_TENSORRT}")
message(STATUS "ONNX Runtime: ${HAVE_ONNXRUNTIME}")
message(STATUS "========================================")
