#include "inference/backend_factory.h"
#include "inference/backends/tensorrt_backend.h"
#include "inference/backends/onnxruntime_backend.h"
#include "utils/one_logger.hpp"

using namespace infer_frame::backend;
using namespace infer_frame::base;

// 声明初始化函数
namespace infer_frame {
namespace backend {
extern void initializeBackends();
}
}

int main() {
    LOG_INFO("======================================");
    LOG_INFO("  Backend Abstraction Layer Test");
    LOG_INFO("======================================");
    
    // 显式初始化后端注册
    infer_frame::backend::initializeBackends();
    
    // 获取工厂实例
    auto& factory = BackendFactory::getInstance();
    
    // 查看支持的后端
    auto supported_backends = factory.getSupportedBackends();
    LOG_INFO("Supported backends count: {}", supported_backends.size());
    for (auto type : supported_backends) {
        LOG_INFO("  - {}", backendTypeToString(type));
    }
    
    // 测试创建 TensorRT 后端
    LOG_INFO("\n--- Testing TensorRT Backend ---");
    BackendConfig trt_config;
    trt_config.backend_type = BackendType::kTensorRT;
    trt_config.model_path = "/path/to/model.engine";
    trt_config.device_id = 0;
    
    auto trt_backend = factory.createBackend(trt_config);
    if (trt_backend) {
        LOG_INFO("✓ TensorRT backend created and initialized");
        LOG_INFO("  Backend name: {}", trt_backend->getName());
        LOG_INFO("  Initialized: {}", trt_backend->isInitialized());
        
        // 测试推理接口（空调用）
        std::vector<Tensor*> inputs;
        std::vector<Tensor*> outputs;
        auto status = trt_backend->infer(inputs, outputs);
        if (status.ok()) {
            LOG_INFO("✓ Inference call succeeded (empty)");
        }
        
        // 反初始化
        trt_backend->deinit();
        LOG_INFO("✓ TensorRT backend deinitialized");
    } else {
        LOG_ERROR("✗ Failed to create TensorRT backend");
    }
    
    // 测试创建 ONNXRuntime 后端
    LOG_INFO("\n--- Testing ONNXRuntime Backend ---");
    BackendConfig onnx_config;
    onnx_config.backend_type = BackendType::kONNXRuntime;
    onnx_config.model_path = "/path/to/model.onnx";
    onnx_config.device_id = 0;
    
    auto onnx_backend = factory.createBackend(onnx_config);
    if (onnx_backend) {
        LOG_INFO("✓ ONNXRuntime backend created and initialized");
        LOG_INFO("  Backend name: {}", onnx_backend->getName());
        LOG_INFO("  Initialized: {}", onnx_backend->isInitialized());
        
        onnx_backend->deinit();
        LOG_INFO("✓ ONNXRuntime backend deinitialized");
    } else {
        LOG_ERROR("✗ Failed to create ONNXRuntime backend");
    }
    
    // 测试不支持的后端
    LOG_INFO("\n--- Testing Unsupported Backend ---");
    BackendConfig unknown_config;
    unknown_config.backend_type = BackendType::kUnknown;
    
    auto unknown_backend = factory.createBackend(unknown_config);
    if (!unknown_backend) {
        LOG_INFO("✓ Correctly rejected unsupported backend");
    }
    
    LOG_INFO("\n======================================");
    LOG_INFO("  All tests completed!");
    LOG_INFO("======================================");
    
    return 0;
}
