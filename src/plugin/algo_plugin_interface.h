#pragma once

/**
 * @file algo_plugin_interface.h
 * @brief 算法插件 C 接口定义（VSE 风格 + NNDeploy Backend 抽象）
 * 
 * 设计目标：
 * 1. 算法可以独立编译为 .so
 * 2. 使用 C 函数导出，避免 C++ ABI 问题
 * 3. Backend 由用户通过参数指定（类似 NNDeploy）
 * 4. 算法内部集成推理引擎（TensorRT/ONNX/OpenVINO）
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

// ============================================================================
// 基础类型定义
// ============================================================================

/**
 * @brief 状态码
 */
typedef enum {
  ALGO_STATUS_SUCCESS = 0,
  ALGO_STATUS_ERROR_INVALID_PARAM = 1,
  ALGO_STATUS_ERROR_NOT_INITIALIZED = 2,
  ALGO_STATUS_ERROR_ALREADY_INITIALIZED = 3,
  ALGO_STATUS_ERROR_OUT_OF_MEMORY = 4,
  ALGO_STATUS_ERROR_FILE_NOT_FOUND = 5,
  ALGO_STATUS_ERROR_MODEL_LOAD = 6,
  ALGO_STATUS_ERROR_INFERENCE = 7,
  ALGO_STATUS_ERROR_BACKEND_NOT_SUPPORTED = 8,
  ALGO_STATUS_ERROR_UNKNOWN = 99
} AlgoStatus;

/**
 * @brief Backend 类型（参考 NNDeploy）
 */
typedef enum {
  ALGO_BACKEND_TENSORRT = 0,      // NVIDIA TensorRT
  ALGO_BACKEND_ONNXRUNTIME = 1,   // ONNX Runtime
  ALGO_BACKEND_OPENVINO = 2,      // Intel OpenVINO
  ALGO_BACKEND_MNN = 3,           // Alibaba MNN
  ALGO_BACKEND_NCNN = 4,          // Tencent ncnn
  ALGO_BACKEND_TNN = 5,           // Tencent TNN
  ALGO_BACKEND_RKNN = 6,          // Rockchip RKNN
  ALGO_BACKEND_ASCENDCL = 7,      // Huawei Ascend CL
  ALGO_BACKEND_COREML = 8,        // Apple CoreML
  ALGO_BACKEND_UNKNOWN = 99
} AlgoBackendType;

/**
 * @brief 算法类型
 */
typedef enum {
  ALGO_TYPE_DETECTION = 0,        // 目标检测
  ALGO_TYPE_CLASSIFICATION = 1,   // 图像分类
  ALGO_TYPE_SEGMENTATION = 2,     // 图像分割
  ALGO_TYPE_OCR = 3,              // 文字识别
  ALGO_TYPE_POSE = 4,             // 姿态估计
  ALGO_TYPE_FACE = 5,             // 人脸识别
  ALGO_TYPE_TRACK = 6,            // 目标跟踪
  ALGO_TYPE_UNKNOWN = 99
} AlgoType;

/**
 * @brief 数据类型
 */
typedef enum {
  ALGO_DATA_TYPE_FLOAT32 = 0,
  ALGO_DATA_TYPE_FLOAT16 = 1,
  ALGO_DATA_TYPE_INT8 = 2,
  ALGO_DATA_TYPE_UINT8 = 3,
  ALGO_DATA_TYPE_INT32 = 4,
  ALGO_DATA_TYPE_UNKNOWN = 99
} AlgoDataType;

/**
 * @brief Tensor 结构
 */
typedef struct {
  char name[64];              // Tensor 名称
  AlgoDataType data_type;     // 数据类型
  int ndim;                   // 维度数量
  int64_t shape[8];           // 最多 8 维
  void* data;                 // 数据指针
  size_t size;                // 数据字节数
} AlgoTensor;

/**
 * @brief 检测框结构
 */
typedef struct {
  float x1, y1, x2, y2;       // 边界框坐标
  float score;                // 置信度
  int class_id;               // 类别 ID
  char class_name[64];        // 类别名称
} AlgoDetBox;

/**
 * @brief 检测结果
 */
typedef struct {
  AlgoDetBox* boxes;          // 检测框数组
  int num_boxes;              // 检测框数量
  int64_t timestamp;          // 时间戳
} AlgoDetResult;

/**
 * @brief 算法信息
 */
typedef struct {
  char name[64];              // 算法名称
  char version[32];           // 版本号
  AlgoType type;              // 算法类型
  char description[256];      // 描述信息
  char author[64];            // 作者
  AlgoBackendType* supported_backends;  // 支持的 Backend 列表
  int num_backends;           // Backend 数量
} AlgoInfo;

/**
 * @brief 初始化参数
 */
typedef struct {
  const char* model_path;      // 模型文件路径
  AlgoBackendType backend;     // Backend 类型
  int device_id;               // 设备 ID（GPU ID）
  const char* config_json;     // JSON 格式的算法参数
} AlgoInitParam;

/**
 * @brief 算法句柄（不透明指针）
 */
typedef void* AlgoHandle;

// ============================================================================
// 插件导出函数（每个算法 .so 必须实现）
// ============================================================================

/**
 * @brief 获取算法信息
 * @return 算法信息结构指针，失败返回 NULL
 */
const AlgoInfo* AlgoGetInfo();

/**
 * @brief 创建算法实例
 * @return 算法句柄，失败返回 NULL
 */
AlgoHandle AlgoCreate();

/**
 * @brief 初始化算法
 * @param handle 算法句柄
 * @param param 初始化参数
 * @return 状态码
 */
AlgoStatus AlgoInit(AlgoHandle handle, const AlgoInitParam* param);

/**
 * @brief 执行推理（目标检测）
 * @param handle 算法句柄
 * @param input 输入 Tensor
 * @param result 检测结果（输出）
 * @return 状态码
 */
AlgoStatus AlgoInferDetection(AlgoHandle handle, const AlgoTensor* input, AlgoDetResult* result);

/**
 * @brief 反初始化
 * @param handle 算法句柄
 * @return 状态码
 */
AlgoStatus AlgoDeinit(AlgoHandle handle);

/**
 * @brief 销毁算法实例
 * @param handle 算法句柄
 */
void AlgoDestroy(AlgoHandle handle);

/**
 * @brief 释放检测结果内存
 * @param result 检测结果
 */
void AlgoFreeDetResult(AlgoDetResult* result);

#ifdef __cplusplus
}
#endif
