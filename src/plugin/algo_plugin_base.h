#pragma once

#include "inference/base/status.h"
#include "inference/base/types.h"
#include "inference/backend_interface.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace infer_frame {
namespace plugin {

/**
 * @brief 算法类型枚举
 */
enum class AlgoType {
  kUnknown = 0,
  kDetection = 1,      // 目标检测
  kSegmentation = 2,   // 语义分割
  kClassification = 3, // 分类
  kOCR = 4,            // 文字识别
  kTracking = 5,       // 目标跟踪
  kPose = 6,           // 姿态估计
  kCustom = 100        // 自定义算法
};

/**
 * @brief 插件元信息
 */
struct AlgoInfo {
  std::string name;                                    // 插件名称，如 "YOLOv8"
  std::string version;                                 // 版本号，如 "1.0.0"
  AlgoType type;                                       // 算法类型
  std::string description;                             // 描述信息
  std::vector<backend::BackendType> supported_backends; // 支持的后端列表
  std::string author;                                  // 作者
  
  AlgoInfo()
      : name("Unknown"),
        version("0.0.0"),
        type(AlgoType::kUnknown),
        description(""),
        author("") {}
};

/**
 * @brief 算法插件基类
 * 
 * 所有算法插件必须继承此类，并实现所有虚函数。
 * 插件以独立 .so 文件形式存在，运行时动态加载。
 * 
 * 使用示例:
 * @code
 * class YOLOv8Plugin : public AlgoPluginBase {
 *  public:
 *   AlgoInfo getInfo() override {
 *     AlgoInfo info;
 *     info.name = "YOLOv8";
 *     info.version = "1.0.0";
 *     info.type = AlgoType::kDetection;
 *     return info;
 *   }
 *   
 *   base::Status init(const std::string& model_path,
 *                     const backend::BackendConfig& backend_config,
 *                     const std::map<std::string, std::string>& algo_params) override {
 *     // 初始化逻辑
 *     return base::Status::OK();
 *   }
 *   // ... 其他实现
 * };
 * 
 * REGISTER_ALGO_PLUGIN(YOLOv8Plugin)
 * @endcode
 */
class AlgoPluginBase {
 public:
  virtual ~AlgoPluginBase() = default;
  
  /**
   * @brief 获取插件信息
   * @return 插件元信息
   */
  virtual AlgoInfo getInfo() const = 0;
  
  /**
   * @brief 初始化插件
   * 
   * @param model_path 模型文件路径
   * @param backend_config 后端配置
   * @param algo_params 算法参数（如 conf_threshold, nms_threshold 等）
   * @return Status 初始化状态
   */
  virtual base::Status init(
      const std::string& model_path,
      const backend::BackendConfig& backend_config,
      const std::map<std::string, std::string>& algo_params) = 0;
  
  /**
   * @brief 单帧推理
   * 
   * @param inputs 输入 Tensor 列表
   * @param outputs 输出 Tensor 列表（插件负责填充）
   * @return Status 推理状态
   */
  virtual base::Status infer(
      std::vector<base::Tensor*>& inputs,
      std::vector<base::Tensor*>& outputs) = 0;
  
  /**
   * @brief 批量推理
   * 
   * @param batch_inputs 批量输入 Tensor
   * @param batch_outputs 批量输出 Tensor（插件负责填充）
   * @return Status 推理状态
   */
  virtual base::Status inferBatch(
      const std::vector<std::vector<base::Tensor*>>& batch_inputs,
      std::vector<std::vector<base::Tensor*>>& batch_outputs) = 0;
  
  /**
   * @brief 反初始化，释放资源
   * @return Status 反初始化状态
   */
  virtual base::Status deinit() = 0;
  
  /**
   * @brief 检查插件是否已初始化
   * @return 是否已初始化
   */
  virtual bool isInitialized() const = 0;
  
  /**
   * @brief 获取插件支持的后端类型
   * @return 支持的后端类型列表
   */
  virtual std::vector<backend::BackendType> getSupportedBackends() const {
    return getInfo().supported_backends;
  }
};

/**
 * @brief 插件工厂函数类型
 * 
 * 每个插件 .so 必须导出一个名为 createAlgoPlugin 的函数，
 * 返回插件实例的智能指针。
 */
using CreateAlgoPluginFunc = std::shared_ptr<AlgoPluginBase> (*)();

}  // namespace plugin
}  // namespace infer_frame
