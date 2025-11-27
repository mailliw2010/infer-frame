#include "inference/backend_factory.h"
#include "inference/backends/tensorrt_backend.h"
#include "inference/backends/onnxruntime_backend.h"

// 注册所有后端
namespace infer_frame {
namespace backend {

// 使用注册宏注册后端
REGISTER_BACKEND(TensorRT, TensorRTBackend)
REGISTER_BACKEND(ONNXRuntime, ONNXRuntimeBackend)

// TODO: 其他后端注册
// REGISTER_BACKEND(RKNN, RKNNBackend)
// REGISTER_BACKEND(Sophon, SophonBackend)
// REGISTER_BACKEND(OpenVINO, OpenVINOBackend)

// 显式初始化函数（确保静态注册被触发）
void initializeBackends() {
  // 这个函数的存在确保此编译单元被链接
  // 静态变量会在此之前被初始化
  LOG_INFO("Backend registry initialized");
}

}  // namespace backend
}  // namespace infer_frame
