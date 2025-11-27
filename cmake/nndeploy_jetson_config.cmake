# --------------------------------------------------------------------
# NNDeploy Config for NVIDIA Jetson Platform
#
# Platform: Jetson Orin (aarch64)
# CUDA: 11.4.206
# TensorRT: 8.5.2.2
# cuDNN: 8.6.0
#
# This config enables:
# - TensorRT inference backend
# - YOLO detection plugin
# - Traditional CV plugins (preprocess, classification, segment, codec)
# - Disables LLM and Stable Diffusion plugins
# --------------------------------------------------------------------

# IR ONNX
set(ENABLE_NNDEPLOY_IR_ONNX OFF) # Support generating IR directly from ONNX models, disabled by default

# Demos and Tests
set(ENABLE_NNDEPLOY_DEMO ON) # Disable NNDeploy official demos
set(ENABLE_NNDEPLOY_TEST ON) # Disable NNDeploy tests

# Device Backend Options (Enable as Needed, All Disabled by Default, No Device Backend Dependencies)
# Note: CUDA device will be auto-enabled by TensorRT backend
# set(ENABLE_NNDEPLOY_DEVICE_CUDA ON) # Don't set explicitly, let NNDeploy auto-detect
set(ENABLE_NNDEPLOY_DEVICE_ROCM OFF) # Whether to enable device ROCM, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_SYCL OFF) # Whether to enable device SYCL, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_OPENCL OFF) # Whether to enable device OpenCL, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_OPENGL OFF) # Whether to enable device OpenGL, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_METAL OFF) # Whether to enable device Metal, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_VULKAN OFF) # Whether to enable device Vulkan, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_HEXAGON OFF) # Whether to enable device Hexagon, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_MTK_VPU OFF) # Whether to enable device MTK VPU, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_ASCEND_CL OFF) # Whether to enable device Ascend CL, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_APPLE_NPU OFF) # Whether to enable device Apple NPU, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_QUALCOMM_NPU OFF) # Whether to enable device Qualcomm NPU, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_MTK_NPU OFF) # Whether to enable device MTK NPU, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_SOPHON_NPU OFF) # Whether to enable device Sophon NPU, default is OFF

# Operator Backend Options (Enable as Needed, All Disabled by Default, No Operator Backend Dependencies)
# Note: Explicitly disable CUDNN to prevent CUDA kernel compilation (TensorRT doesn't need it)
set(ENABLE_NNDEPLOY_DEVICE_CUDNN OFF) # Disable CUDNN to avoid CUDA kernel compilation errors
set(ENABLE_NNDEPLOY_DEVICE_X86_ONEDNN OFF) # Whether to enable operator X86_ONEDNN, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_ARM_XNNPACK OFF) # Whether to enable operator ARM_XNNPACK, default is OFF
set(ENABLE_NNDEPLOY_DEVICE_ARM_QNNPACK OFF) # Whether to enable operator ARM_QNNPACK, default is OFF

# Inference Backend Options (Enable as Needed, All Disabled by Default, No Inference Backend Dependencies)
set(ENABLE_NNDEPLOY_INFERENCE_TENSORRT ON) # Whether to enable INFERENCE TENSORRT, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_OPENVINO OFF) # Whether to enable INFERENCE OPENVINO, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_COREML OFF) # Whether to enable INFERENCE COREML, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_TFLITE OFF) # Whether to enable INFERENCE TFLITE, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_ONNXRUNTIME OFF) # Whether to enable INFERENCE ONNXRUNTIME, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_NCNN OFF) # Whether to enable INFERENCE NCNN, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_TNN OFF) # Whether to enable INFERENCE TNN, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_MNN OFF) # Whether to enable INFERENCE MNN, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_TVM OFF) # Whether to enable INFERENCE TVM, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_PADDLELITE OFF) # Whether to enable INFERENCE PADDLELITE, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_RKNN_TOOLKIT_1 OFF) # Whether to enable INFERENCE RKNN_TOOLKIT_1, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_RKNN_TOOLKIT_2 OFF) # Whether to enable INFERENCE RKNN_TOOLKIT_2, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_ASCEND_CL OFF) # Whether to enable INFERENCE ASCEND_CL, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_SNPE OFF) # Whether to enable INFERENCE SNPE, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_QNN OFF) # Whether to enable INFERENCE QNN, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_SOPHON OFF) # Whether to enable INFERENCE SOPHON, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_TORCH OFF) # Whether to enable INFERENCE TORCH, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_TENSORFLOW OFF) # Whether to enable INFERENCE TENSORFLOW, default is OFF
set(ENABLE_NNDEPLOY_INFERENCE_NEUROPILOT OFF) # Whether to enable INFERENCE NEUROPILOT, default is OFF

# Algorithm Plugin Options (Recommended to use default configuration, traditional CV algorithms enabled, language and text-to-image algorithms disabled by default)
## OpenCV
# set(ENABLE_NNDEPLOY_OPENCV "path/to/opencv") # 通过路径的方式链接OpenCV
# set(NNDEPLOY_OPENCV_LIBS "opencv_world4100") # Specific OpenCV library names to link, such as opencv_world4100, opencv_java4, etc.
set(ENABLE_NNDEPLOY_OPENCV ON) # Whether to link the third-party OpenCV library, default is ON
set(NNDEPLOY_OPENCV_LIBS) # Link all OpenCV libraries by default

## RapidJSON (Required for DAG/Graph based inference like YoloGraph)
set(ENABLE_NNDEPLOY_RAPIDJSON ON) # Enable RapidJSON for graph-based inference

## Traditional CV Algorithm Plugins
set(ENABLE_NNDEPLOY_PLUGIN ON) # Enable plugin system
set(ENABLE_NNDEPLOY_PLUGIN_DETECT ON) # Enable detection plugin
set(ENABLE_NNDEPLOY_PLUGIN_DETECT_YOLO ON) # Enable YOLO detection
set(ENABLE_NNDEPLOY_PLUGIN_PREPROCESS ON) # Enable preprocess plugin
set(ENABLE_NNDEPLOY_PLUGIN_INFER ON) # Enable inference plugin
set(ENABLE_NNDEPLOY_PLUGIN_CLASSIFICATION ON) # Enable classification plugin
set(ENABLE_NNDEPLOY_PLUGIN_SEGMENT ON) # Enable segmentation plugin
set(ENABLE_NNDEPLOY_PLUGIN_CODEC ON) # Enable codec plugin

## Tokenizer-cpp
set(ENABLE_NNDEPLOY_PLUGIN_TOKENIZER_CPP OFF) # Whether to enable C++ tokenizer plugin, default is OFF

## Language Model
set(ENABLE_NNDEPLOY_PLUGIN_LLM OFF) # Whether to enable language model plugin, default is OFF

## Stable Diffusion
set(ENABLE_NNDEPLOY_PLUGIN_STABLE_DIFFUSION OFF) # Whether to enable text-to-image plugin, default is OFF