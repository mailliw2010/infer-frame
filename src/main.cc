#include <iostream>
#include <memory>
#include <string>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#include "utils/one_logger.hpp"

// 定义构建类型字符串
#ifdef CMAKE_BUILD_TYPE
    #define STRINGIFY(x) #x
    #define TOSTRING(x) STRINGIFY(x)
    #define CMAKE_BUILD_TYPE_STRING TOSTRING(CMAKE_BUILD_TYPE)
#else
    #define CMAKE_BUILD_TYPE_STRING "Unknown"
#endif

// TODO: 当 gRPC 代码生成后取消注释
// #include "infer_service.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

// 全局停止标志
std::atomic<bool> g_stop_flag(false);

// 信号处理函数
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        LOG_INFO("Received signal {}, shutting down gracefully...", signal);
        g_stop_flag.store(true);
    }
}

// TODO: InferenceServiceImpl 实现将在 src/grpc_service/ 中
// 这里先提供一个占位类
class InferenceServiceImpl {
public:
    InferenceServiceImpl() {
        LOG_INFO("InferenceService initialized");
    }
    
    ~InferenceServiceImpl() {
        LOG_INFO("InferenceService destroyed");
    }
};

class InferFrameServer {
public:
    InferFrameServer(const std::string& server_address)
        : server_address_(server_address) {}
    
    void run() {
        LOG_INFO("======================================");
        LOG_INFO("  Infer-Frame Server v1.0.0");
        LOG_INFO("  High-Performance Inference Engine");
        LOG_INFO("======================================");
        LOG_INFO("Platform: {}", getPlatformInfo());
        LOG_INFO("Build Type: {}", CMAKE_BUILD_TYPE_STRING);
        
        // 初始化推理服务
        service_impl_ = std::make_unique<InferenceServiceImpl>();
        
        // TODO: 当 proto 编译完成后启用 gRPC 服务器
        // ServerBuilder builder;
        // builder.AddListeningPort(server_address_, 
        //                          grpc::InsecureServerCredentials());
        // builder.RegisterService(service_impl_.get());
        
        // // 启用健康检查
        // grpc::EnableDefaultHealthCheckService(true);
        // grpc::reflection::InitProtoReflectionServerBuilderPlugin();
        
        // // 构建并启动服务器
        // server_ = builder.BuildAndStart();
        
        LOG_INFO("Server listening on {}", server_address_);
        LOG_INFO("Press Ctrl+C to stop");
        
        // 主循环（暂时用简单的 sleep）
        while (!g_stop_flag.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        // TODO: server_->Shutdown();
        LOG_INFO("Server stopped");
    }
    
private:
    std::string getPlatformInfo() {
#if defined(JETSON_PLATFORM)
        return "NVIDIA Jetson (TensorRT)";
#elif defined(RKNN_PLATFORM)
        return "Rockchip RKNN";
#elif defined(SOPHON_PLATFORM)
        return "Sophon BM1684X";
#elif defined(ENABLE_CUDA)
        return "x86_64 CUDA (TensorRT)";
#else
        return "Generic CPU";
#endif
    }
    
    std::string server_address_;
    std::unique_ptr<InferenceServiceImpl> service_impl_;
    std::unique_ptr<Server> server_;
};

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "Options:\n"
              << "  --grpc_port PORT         gRPC server port (default: 50051)\n"
              << "  --config PATH            Config file path\n"
              << "  --plugin_dir PATH        Plugin directory\n"
              << "  --log_level LEVEL        Log level (DEBUG|INFO|WARN|ERROR)\n"
              << "  --help                   Show this help message\n"
              << std::endl;
}

int main(int argc, char** argv) {
    // 默认参数
    std::string grpc_port = "50051";
    std::string config_path = "/etc/infer-frame/engine_config.json";
    std::string plugin_dir = "/usr/local/lib/infer-frame/plugins";
    std::string log_level = "INFO";
    
    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "--grpc_port" && i + 1 < argc) {
            grpc_port = argv[++i];
        } else if (arg == "--config" && i + 1 < argc) {
            config_path = argv[++i];
        } else if (arg == "--plugin_dir" && i + 1 < argc) {
            plugin_dir = argv[++i];
        } else if (arg == "--log_level" && i + 1 < argc) {
            log_level = argv[++i];
        } else {
            LOG_ERROR("Unknown argument: {}", arg);
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // 注册信号处理
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // 构建服务器地址
    std::string server_address = "0.0.0.0:" + grpc_port;
    
    try {
        // 创建并运行服务器
        InferFrameServer server(server_address);
        server.run();
        
        return 0;
    } catch (const std::exception& e) {
        LOG_ERROR("Exception: {}", e.what());
        return 1;
    }
}

// 定义构建类型字符串在文件开头已经定义
