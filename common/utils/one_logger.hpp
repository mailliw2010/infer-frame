//
// Created by CISDI on 2024/10/21.
//

#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async_logger.h>
#include <spdlog/async.h>
#include <memory>
#include <mutex>
#include <dirent.h>
#include <any>

// 自定义 formatter<std::any>
template <>
struct fmt::formatter<std::any>{
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()){
        return ctx.begin();
    }

    template<typename FormatContext>
    auto format(const std::any& value, FormatContext& ctx) -> decltype(ctx.out()){
        if (!value.has_value()){
            return fmt::format_to(ctx.out(), "null");
        }
        try {
            if (value.type() == typeid(int)){
                return fmt::format_to(ctx.out(), "{}", std::any_cast<int>(value));
            }
            else if (value.type() == typeid(float)){
                return fmt::format_to(ctx.out(), "{}", std::any_cast<float>(value));
            }
            else if (value.type() == typeid(double)){
                return fmt::format_to(ctx.out(), "{}", std::any_cast<double>(value));
            }
            else if (value.type() == typeid(std::string)){
                return fmt::format_to(ctx.out(), "{}", std::any_cast<std::string>(value));
            }
        } catch (const std::bad_any_cast&){
            return fmt::format_to(ctx.out(), "[bad_any_cast]");
        }
        return fmt::format_to(ctx.out(), "[unsupported type: {}]", value.type().name());
    }
};


class OneLogger {
public:
    // 获取单例实例
    static spdlog::logger* getInstance() {
        static std::once_flag initFlag;
        // 使用局部静态变量实现打到哪里模式，避免在类外初始化静态成员变量(类外初始化必须放在cpp中不够优雅)
        static std::shared_ptr<spdlog::logger> logger_;
        std::call_once(initFlag, []() {
            // 创建控制台输出和文件输出的日志器
            std::vector<spdlog::sink_ptr> sinks;

            // 彩色输出到控制台
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::debug);
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%^%l%$][%t][%s:%#] %v");
            sinks.push_back(console_sink);

            // 输出到文件
            // 每个日志文件最大 20MB，最多保留 10 个日志文件
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("log/app.log", 1024*1024*20, 10);
            file_sink->set_level(spdlog::level::debug);
            sinks.push_back(file_sink);

            // 创建异步线程池，最多缓存 8192 条日志
            spdlog::init_thread_pool(8192, 1);  // 1 个后台线程

            // 创建异步日志器
            logger_ = std::make_shared<spdlog::async_logger>(
                    "multi_sink", sinks.begin(), sinks.end(), spdlog::thread_pool(),
                    spdlog::async_overflow_policy::block);  // 使用 block 策略，当队列满时阻塞

            spdlog::register_logger(logger_);
            // 设置日志的格式，包含时间戳、日志级别、线程ID、文件名、行号和消息
            logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%^%l%$][%t][%s:%#] %v");
            logger_->set_level(spdlog::level::debug); // 设置全局日志级别
            logger_->flush_on(spdlog::level::warn);   // warn/error 级别立即刷新到磁盘
            
            // 高频调用场景：启用每2秒定期刷新，确保 debug/info 日志也能及时写入文件
            // 避免动态库环境下因缓冲未刷新导致日志丢失
            spdlog::flush_every(std::chrono::seconds(2));
        });

        return logger_.get();
    }

    // 禁止拷贝和赋值
    OneLogger(const OneLogger&) = delete;
    OneLogger& operator=(const OneLogger&) = delete;

private:
    OneLogger() = default; // 私有构造函数
};


//// 定义简化的日志宏
#define one_logger OneLogger::getInstance()
#define log_info(...)    one_logger->log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::info, __VA_ARGS__)
#define log_error(...)   one_logger->log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::err,  __VA_ARGS__)
#define log_warn(...)    one_logger->log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::warn, __VA_ARGS__)
#define log_debug(...)   one_logger->log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::debug, __VA_ARGS__)

#define LOG_DEBUG(...)    one_logger->log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(...)    one_logger->log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::info, __VA_ARGS__)
#define LOG_ERROR(...)   one_logger->log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::err,  __VA_ARGS__)
#define LOG_WARN(...)    one_logger->log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::warn, __VA_ARGS__)
#define LOG_WARNING(...)    one_logger->log(spdlog::source_loc{__FILE__, __LINE__, __func__}, spdlog::level::warn, __VA_ARGS__)
