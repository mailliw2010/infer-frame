cmake_minimum_required(VERSION 3.10)
project(infer-frame VERSION 1.0.0 LANGUAGES CXX C)

# C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# 编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g -O0 -Wall -Wextra)
    add_definitions(-DDEBUG)
else()
    add_compile_options(-O3 -DNDEBUG)
endif()

# 定义构建类型宏，供 main.cpp 使用
add_definitions(-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

# 定义构建类型宏，供 main.cpp 使用
add_definitions(-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE})

# 平台检测和工具链加载
message(STATUS "Detecting platform...")
if(CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
    include(${CMAKE_TOOLCHAIN_FILE})
else()
    # 自动检测平台
    if(EXISTS "/proc/device-tree/model")
        file(READ "/proc/device-tree/model" DEVICE_MODEL)
        if(DEVICE_MODEL MATCHES "Jetson")
            set(PLATFORM "JETSON")
            message(STATUS "Auto-detected: Jetson platform")
            include(${CMAKE_CURRENT_SOURCE_DIR}/platforms/aarch64_jetson.cmake)
        endif()
    else()
        set(PLATFORM "X86_64")
        message(STATUS "Using default: x86_64 platform")
    endif()
endif()

# 查找依赖
message(STATUS "Finding dependencies...")

# Protobuf 和 gRPC
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG)
if(NOT gRPC_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GRPC REQUIRED grpc++)
    pkg_check_modules(GRPCPP grpc++>=1.16.0)
    if(NOT GRPCPP_FOUND)
        set(GRPCPP_LIBRARIES ${GRPC_LIBRARIES})
        set(GRPCPP_INCLUDE_DIRS ${GRPC_INCLUDE_DIRS})
    endif()
    set(GRPC_LIBRARIES ${GRPC_LIBRARIES} ${GRPCPP_LIBRARIES})
    set(GRPC_INCLUDE_DIRS ${GRPC_INCLUDE_DIRS} ${GRPCPP_INCLUDE_DIRS})
    # 查找 grpc_cpp_plugin
    find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)
    if(NOT GRPC_CPP_PLUGIN)
        message(FATAL_ERROR "grpc_cpp_plugin not found")
    endif()
else()
    set(GRPC_LIBRARIES gRPC::grpc++)
    set(GRPC_CPP_PLUGIN $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
endif()

# GStreamer
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0)
pkg_check_modules(GSTREAMER_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GSTREAMER_VIDEO REQUIRED gstreamer-video-1.0)

# CUDA (for TensorRT backend)
find_package(CUDA QUIET)
if(CUDA_FOUND)
    message(STATUS "Found CUDA: ${CUDA_VERSION}")
    include_directories(${CUDA_INCLUDE_DIRS})
endif()

# OpenCV
find_package(OpenCV REQUIRED)

# spdlog (使用 3rdparty 中的版本)
set(SPDLOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/spdlog)
if(EXISTS ${SPDLOG_DIR}/include)
    message(STATUS "Using spdlog from 3rdparty: ${SPDLOG_DIR}")
    include_directories(${SPDLOG_DIR}/include)
    set(SPDLOG_FOUND TRUE)
else()
    message(WARNING "spdlog not found in 3rdparty/spdlog")
    set(SPDLOG_FOUND FALSE)
endif()

# nlohmann-json (可选，用于配置文件解析)
set(JSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/nlohmann-json)
if(EXISTS ${JSON_DIR}/include)
    message(STATUS "Using nlohmann-json from 3rdparty: ${JSON_DIR}")
    include_directories(${JSON_DIR}/include)
endif()

# NNDeploy（核心推理框架）
set(NNDEPLOY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/nndeploy)
if(EXISTS ${NNDEPLOY_DIR})
    message(STATUS "Found NNDeploy: ${NNDEPLOY_DIR}")
    
    # 添加 NNDeploy 子项目（移除 EXCLUDE_FROM_ALL，使其自动编译）
    option(ENABLE_NNDEPLOY_BUILD_SHARED "Build NNDeploy as shared library" ON)
    add_subdirectory(${NNDEPLOY_DIR})
    # 包含 NNDeploy 头文件
    include_directories(${NNDEPLOY_DIR}/framework/include)
    include_directories(${NNDEPLOY_DIR}/plugin/include)
    set(NNDEPLOY_FOUND TRUE)
else()
    message(WARNING "NNDeploy not found in 3rdparty/nndeploy")
    set(NNDEPLOY_FOUND FALSE)
endif()

# NNDeploy (暂时跳过，待完善)
set(NNDEPLOY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/nndeploy)
option(ENABLE_NNDEPLOY "Enable NNDeploy integration" OFF)
if(ENABLE_NNDEPLOY)
    if(EXISTS ${NNDEPLOY_DIR}/CMakeLists.txt)
        message(STATUS "NNDeploy found, will integrate later")
        # TODO: add_subdirectory(${NNDEPLOY_DIR})
    else()
        message(WARNING "NNDeploy not found. Please run: git submodule update --init --recursive")
    endif()
endif()

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_BINARY_DIR}  # 生成的 proto 文件
    ${PROTOBUF_INCLUDE_DIRS}
    ${GRPC_INCLUDE_DIRS}
    ${GSTREAMER_INCLUDE_DIRS}
    ${GSTREAMER_APP_INCLUDE_DIRS}
    ${GSTREAMER_VIDEO_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

# 生成 Protobuf 和 gRPC 代码
set(PROTO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/proto")
file(GLOB PROTO_FILES "${PROTO_PATH}/*.proto")

set(PROTO_SRCS "")
set(PROTO_HDRS "")

if(PROTO_FILES)
    foreach(PROTO_FILE ${PROTO_FILES})
        get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
        set(PROTO_SRC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.cc")
        set(PROTO_HDR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.pb.h")
        set(GRPC_SRC "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.cc")
        set(GRPC_HDR "${CMAKE_CURRENT_BINARY_DIR}/${PROTO_NAME}.grpc.pb.h")
        
        add_custom_command(
            OUTPUT ${PROTO_SRC} ${PROTO_HDR} ${GRPC_SRC} ${GRPC_HDR}
            COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
            ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
                 --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
                 --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
                 -I${PROTO_PATH}
                 ${PROTO_FILE}
            DEPENDS ${PROTO_FILE}
            COMMENT "Generating protobuf and gRPC code for ${PROTO_NAME}"
        )
        
        list(APPEND PROTO_SRCS ${PROTO_SRC} ${GRPC_SRC})
        list(APPEND PROTO_HDRS ${PROTO_HDR} ${GRPC_HDR})
    endforeach()
    
    message(STATUS "Found ${CMAKE_CURRENT_LIST_LINE} proto files, will generate gRPC code")
else()
    message(WARNING "No proto files found in ${PROTO_PATH}")
endif()

# 源文件 (使用 .cc 后缀)
file(GLOB_RECURSE CODEC_SRCS "src/codec/*.cc")
file(GLOB_RECURSE INFERENCE_SRCS "src/inference/*.cc")
file(GLOB_RECURSE GRPC_SERVICE_SRCS "src/grpc_service/*.cc")

# 排除测试文件和示例程序
list(FILTER INFERENCE_SRCS EXCLUDE REGEX ".*_test\\.cc$")
list(FILTER INFERENCE_SRCS EXCLUDE REGEX ".*_demo\\.cc$")
list(FILTER INFERENCE_SRCS EXCLUDE REGEX ".*_nndeploy\\.cc$")

# 暂时创建一个空的核心库（因为子目录还是空的）
if(PROTO_SRCS)
    # 主程序库
    add_library(infer_frame_core STATIC
        ${PROTO_SRCS}
    )
    
    # 如果有其他源文件，也添加进来
    if(CODEC_SRCS)
        target_sources(infer_frame_core PRIVATE ${CODEC_SRCS})
    endif()
    if(INFERENCE_SRCS)
        target_sources(infer_frame_core PRIVATE ${INFERENCE_SRCS})
    endif()
    if(GRPC_SERVICE_SRCS)
        target_sources(infer_frame_core PRIVATE ${GRPC_SERVICE_SRCS})
    endif()
    
    target_link_libraries(infer_frame_core
        PUBLIC
            ${PROTOBUF_LIBRARIES}
            ${GRPC_LIBRARIES}
        PRIVATE
            ${GSTREAMER_LIBRARIES}
            ${GSTREAMER_APP_LIBRARIES}
            ${GSTREAMER_VIDEO_LIBRARIES}
            ${OpenCV_LIBS}
    )
    
    # 链接 NNDeploy
    if(NNDEPLOY_FOUND)
        target_link_libraries(infer_frame_core PUBLIC nndeploy_framework)
        message(STATUS "NNDeploy linked to infer_frame_core")
    endif()
endif()

# 主程序
add_executable(infer_frame_server src/main.cc)

if(TARGET infer_frame_core)
    target_link_libraries(infer_frame_server 
        PRIVATE
            infer_frame_core
            ${GRPC_LIBRARIES}
            ${PROTOBUF_LIBRARIES}
    )
else()
    # 如果核心库还没有，直接链接必要的库
    target_link_libraries(infer_frame_server 
        PRIVATE
            ${GRPC_LIBRARIES}
            ${PROTOBUF_LIBRARIES}
    )
endif()

# 链接线程库
find_package(Threads REQUIRED)
target_link_libraries(infer_frame_server PRIVATE Threads::Threads)

# spdlog 是 header-only 库，不需要额外链接

# 添加 backend_test 可执行文件（可选）
option(BUILD_BACKEND_TEST "Build backend test program" ON)
if(BUILD_BACKEND_TEST AND TARGET infer_frame_core)
    add_executable(backend_test src/inference/backend_test.cc)
    target_link_libraries(backend_test 
        PRIVATE
            infer_frame_core
            Threads::Threads
    )
    message(STATUS "Backend test program will be built")
    
    # YOLOv8 ONNX 推理测试
    add_executable(yolov8_onnx_test src/inference/yolov8_onnx_test.cc)
    target_link_libraries(yolov8_onnx_test 
        PRIVATE
            infer_frame_core
            ${OpenCV_LIBS}
            Threads::Threads
    )
    message(STATUS "YOLOv8 ONNX test program will be built")
    
    # YOLOv8 NNDeploy 原生 API 测试 (暂时禁用，API 不匹配)
    # add_executable(yolov8_nndeploy_test src/inference/yolov8_onnx_test_nndeploy.cc)
    # target_include_directories(yolov8_nndeploy_test 
    #     PRIVATE
    #         ${CMAKE_SOURCE_DIR}/3rdparty/nndeploy/third_party/rapidjson/include
    # )
    # target_link_libraries(yolov8_nndeploy_test 
    #     PRIVATE
    #         nndeploy_framework
    #         ${OpenCV_LIBS}
    #         Threads::Threads
    # )
    # message(STATUS "YOLOv8 NNDeploy test program will be built")
    
    # YOLOv8 目标检测示例（使用 NNDeploy YoloGraph）
    add_executable(yolov8_detect_demo src/inference/yolov8_detect_demo.cc)
    target_include_directories(yolov8_detect_demo 
        PRIVATE
            ${CMAKE_SOURCE_DIR}/3rdparty/nndeploy/third_party/rapidjson/include
    )
    target_compile_definitions(yolov8_detect_demo 
        PRIVATE
            ENABLE_NNDEPLOY_RAPIDJSON
    )
    target_link_libraries(yolov8_detect_demo 
        PRIVATE
            nndeploy_framework
            nndeploy_plugin_detect
            nndeploy_plugin_preprocess
            nndeploy_plugin_infer
            ${OpenCV_LIBS}
            Threads::Threads
    )
    message(STATUS "YOLOv8 detection demo will be built")
endif()

# 添加 plugin_test 可执行文件（可选）
option(BUILD_PLUGIN_TEST "Build plugin test program" ON)
if(BUILD_PLUGIN_TEST AND TARGET infer_frame_core)
    add_executable(plugin_test src/plugin/plugin_test.cc)
    target_link_libraries(plugin_test 
        PRIVATE
            infer_frame_core
            Threads::Threads
            ${CMAKE_DL_LIBS}  # dlopen/dlsym 需要
    )
    message(STATUS "Plugin test program will be built")
    
    # C 接口测试
    add_executable(plugin_test_c src/plugin/plugin_test_c.cc)
    target_link_libraries(plugin_test_c 
        PRIVATE
            infer_frame_core
            Threads::Threads
            ${CMAKE_DL_LIBS}
    )
    message(STATUS "Plugin C interface test program will be built")
endif()

# 插件编译
option(BUILD_PLUGINS "Build algorithm algorithm" ON)
if(BUILD_PLUGINS)
    # YOLOv8 插件（旧版 C++ 接口）
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/yolov8/yolov8_plugin.cc)
        add_library(yolov8_plugin SHARED
            algorithm/yolov8/yolov8_plugin.cc
        )
        target_include_directories(yolov8_plugin PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/common
        )
        target_link_libraries(yolov8_plugin PRIVATE
            infer_frame_core
        )
        # 设置输出目录
        set_target_properties(yolov8_plugin PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/algorithm
            PREFIX ""  # 去掉 lib 前缀
        )
        install(TARGETS yolov8_plugin 
                LIBRARY DESTINATION lib/infer-frame/algorithm)
        message(STATUS "YOLOv8 plugin will be built")
    endif()
    
    # YOLOv8 C 接口插件（新版，独立编译）
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/yolov8/yolov8_plugin_c.cpp)
        add_subdirectory(algorithm/yolov8)
        message(STATUS "YOLOv8 C plugin will be built (standalone)")
    endif()
    
    # PaddleOCR 插件（暂未实现）
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/paddleocr/paddleocr_plugin.cc)
        add_library(paddleocr_plugin SHARED
            algorithm/paddleocr/paddleocr_plugin.cc
        )
        target_include_directories(paddleocr_plugin PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/common
        )
        target_link_libraries(paddleocr_plugin PRIVATE
            infer_frame_core
        )
        set_target_properties(paddleocr_plugin PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/algorithm
            PREFIX ""
        )
        install(TARGETS paddleocr_plugin 
                LIBRARY DESTINATION lib/infer-frame/algorithm)
        message(STATUS "PaddleOCR plugin will be built")
    endif()
endif()

# 安装规则
install(TARGETS infer_frame_server 
        RUNTIME DESTINATION bin)

# 安装配置文件（如果存在）
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config/)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/config/ 
            DESTINATION etc/infer-frame
            FILES_MATCHING PATTERN "*.json" PATTERN "*.yaml")
endif()

# 安装 proto 文件
if(PROTO_FILES)
    install(FILES ${PROTO_FILES} 
            DESTINATION share/infer-frame/proto)
endif()

# 安装平台配置文件
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/platforms/
        DESTINATION share/infer-frame/platforms
        FILES_MATCHING PATTERN "*.cmake")

# 单元测试（可选）
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
        add_subdirectory(tests)
    else()
        message(WARNING "Tests directory not found")
    endif()
endif()

# 打印配置摘要
message(STATUS "========================================")
message(STATUS "  Infer-Frame Configuration Summary")
message(STATUS "========================================")
message(STATUS "Platform: ${TARGET_PLATFORM}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Protobuf: ${PROTOBUF_LIBRARIES}")
message(STATUS "gRPC: ${GRPC_LIBRARIES}")
message(STATUS "OpenCV: ${OpenCV_VERSION}")
message(STATUS "GStreamer: ${GSTREAMER_VERSION}")
message(STATUS "Build Plugins: ${BUILD_PLUGINS}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "========================================")
