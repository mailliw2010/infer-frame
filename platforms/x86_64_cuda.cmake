# x86_64 CUDA Platform Configuration
# For x86_64 Linux systems with NVIDIA CUDA support

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(ARCH "x86_64")
set(PLATFORM "x86_64")
set(TARGET_PLATFORM "x86_64_cuda" CACHE STRING "")

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -Wall -fopenmp" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -fopenmp -DNDEBUG" CACHE STRING "")

# CUDA support
option(ENABLE_CUDA "Enable CUDA support" ON)
if(ENABLE_CUDA)
    find_package(CUDA REQUIRED)
    if(CUDA_FOUND)
        message(STATUS "CUDA found: ${CUDA_VERSION}")
        include_directories(${CUDA_INCLUDE_DIRS})
        set(CUDA_LIBRARIES ${CUDA_LIBRARIES} ${CUDA_CUDART_LIBRARY})
        add_definitions(-DENABLE_CUDA)
    endif()
endif()

# TensorRT support
option(ENABLE_TENSORRT "Enable TensorRT support" ON)
if(ENABLE_TENSORRT)
    set(TENSORRT_ROOT "/usr/local/tensorrt" CACHE PATH "TensorRT installation directory")
    if(EXISTS "${TENSORRT_ROOT}")
        include_directories(${TENSORRT_ROOT}/include)
        link_directories(${TENSORRT_ROOT}/lib)
        set(TENSORRT_LIBRARIES nvinfer nvinfer_plugin nvparsers nvonnxparser)
        add_definitions(-DENABLE_TENSORRT)
        message(STATUS "TensorRT found: ${TENSORRT_ROOT}")
    else()
        message(WARNING "TensorRT not found at ${TENSORRT_ROOT}")
    endif()
endif()

# OpenCV with CUDA (optional)
find_package(OpenCV REQUIRED)
if(OpenCV_CUDA_VERSION)
    message(STATUS "OpenCV with CUDA support: ${OpenCV_CUDA_VERSION}")
    add_definitions(-DOPENCV_CUDA)
endif()
