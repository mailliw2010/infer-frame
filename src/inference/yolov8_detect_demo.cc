/**
 * @file yolov8_detect_demo.cc
 * @brief YOLOv8 目标检测示例（使用 NNDeploy YoloGraph）
 */

#include <opencv2/opencv.hpp>
#include <iostream>
#include "nndeploy/detect/yolo/yolo.h"
#include "nndeploy/detect/result.h"
#include "nndeploy/dag/edge.h"
#include "nndeploy/dag/node.h"
#include "nndeploy/base/glic_stl_include.h"
#include "nndeploy/base/log.h"

using namespace nndeploy;

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "  YOLOv8 Object Detection Demo" << std::endl;
    std::cout << "  Using NNDeploy YoloGraph" << std::endl;
    std::cout << "========================================" << std::endl;

    // 配置参数
    std::string model_path = "/home/mic-711/xcd/infer-frame/algorithm/yolov8/model/yolov8s_quant.onnx";
    std::string image_path = "/home/mic-711/xcd/infer-frame/algorithm/yolov8/data/pic.png";
    std::string output_path = "/home/mic-711/xcd/infer-frame/algorithm/yolov8/data/output.jpg";
    
    // 允许命令行参数覆盖
    if (argc >= 2) image_path = argv[1];
    if (argc >= 3) output_path = argv[2];
    if (argc >= 4) model_path = argv[3];
    
    std::cout << "\n[Config]" << std::endl;
    std::cout << "  Model: " << model_path << std::endl;
    std::cout << "  Input: " << image_path << std::endl;
    std::cout << "  Output: " << output_path << std::endl;

    // 读取图像
    cv::Mat image = cv::imread(image_path);
    if (image.empty()) {
        std::cerr << "\n[ERROR] Failed to load image: " << image_path << std::endl;
        return -1;
    }
    std::cout << "\n[Image] Loaded: " << image.cols << "x" << image.rows << std::endl;

    // 创建 YOLO Graph
    auto yolo_graph = std::make_shared<detect::YoloGraph>("yolo_v8_graph");
    
    // 设置默认参数
    yolo_graph->defaultParam();
    
    // 设置推理类型为 TensorRT
    base::InferenceType inference_type = base::kInferenceTypeTensorRt;
    
    // 设置推理参数
    base::DeviceType device_type;
    device_type.code_ = base::kDeviceTypeCodeCuda;
    device_type.device_id_ = 0;
    
    base::ModelType model_type = base::kModelTypeOnnx;
    bool is_path = true;
    std::vector<std::string> model_value = {model_path};
    
    // 使用 make 方法建立 Graph（关键！）
    dag::NodeDesc pre_desc, infer_desc, post_desc;
    auto status = yolo_graph->make(pre_desc, infer_desc, inference_type, post_desc);
    if (status != base::kStatusCodeOk) {
        std::cerr << "[ERROR] Failed to make YoloGraph: " << status.desc() << std::endl;
        return -1;
    }
    
    // 设置推理参数（在 make 之后）
    yolo_graph->setInferParam(device_type, model_type, is_path, model_value);
    
    // 设置后处理参数
    yolo_graph->setVersion(8);  // YOLOv8
    yolo_graph->setScoreThreshold(0.25f);
    yolo_graph->setNmsThreshold(0.45f);
    yolo_graph->setNumClasses(80);  // COCO 80类
    yolo_graph->setModelHW(640, 640);
    
    std::cout << "\n[Model Config]" << std::endl;
    std::cout << "  Inference: TensorRT" << std::endl;
    std::cout << "  Device: CUDA:0" << std::endl;
    std::cout << "  Version: YOLOv8" << std::endl;
    std::cout << "  Input size: 640x640" << std::endl;
    std::cout << "  Score threshold: 0.25" << std::endl;
    std::cout << "  NMS threshold: 0.45" << std::endl;

    // 初始化
    std::cout << "\n[Init] Initializing YoloGraph..." << std::endl;
    status = yolo_graph->init();
    if (status != base::kStatusCodeOk) {
        std::cerr << "[ERROR] Failed to initialize YoloGraph: " 
                  << status.desc() << std::endl;
        return -1;
    }
    std::cout << "[Init] ✓ YoloGraph initialized successfully" << std::endl;
    
    // 运行推理
    std::cout << "\n[Inference] Running detection..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    
    // 使用 operator() 传入输入图像
    dag::Edge input_edge("input");
    input_edge.set(image);
    auto outputs = (*yolo_graph)(&input_edge);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    if (outputs.empty()) {
        std::cerr << "[ERROR] Detection failed: no output" << std::endl;
        return -1;
    }
    
    std::cout << "[Inference] ✓ Detection completed in " << duration.count() << " ms" << std::endl;

    // 获取检测结果
    detect::DetectResult* detect_result = outputs[0]->getGraphOutput<detect::DetectResult>();
    
    if (!detect_result || detect_result->bboxs_.empty()) {
        std::cerr << "[WARN] No objects detected" << std::endl;
        return 0;
    }

    // 输出检测结果
    std::cout << "\n[Results] Detected " << detect_result->bboxs_.size() << " objects:" << std::endl;
    
    // COCO 类别名称
    std::vector<std::string> class_names = {
        "person", "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
        "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
        "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
        "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
        "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
        "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
        "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
        "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
        "hair drier", "toothbrush"
    };
    
    // 绘制结果
    cv::Mat result_img = image.clone();
    
    for (size_t i = 0; i < detect_result->bboxs_.size(); ++i) {
        const auto& bbox = detect_result->bboxs_[i];
        int label_id = bbox.label_id_;
        float score = bbox.score_;
        
        std::string label = label_id < class_names.size() ? 
                           class_names[label_id] : "unknown";
        
        std::cout << "  [" << i << "] " << label 
                  << " (" << std::fixed << std::setprecision(2) << score * 100 << "%) "
                  << "at [" << bbox.bbox_[0] << ", " << bbox.bbox_[1] 
                  << ", " << bbox.bbox_[2] << ", " << bbox.bbox_[3] << "]" 
                  << std::endl;
        
        // 绘制边界框
        cv::Rect rect(bbox.bbox_[0], bbox.bbox_[1],
                     bbox.bbox_[2] - bbox.bbox_[0],
                     bbox.bbox_[3] - bbox.bbox_[1]);
        cv::rectangle(result_img, rect, cv::Scalar(0, 255, 0), 2);
        
        // 绘制标签
        std::string text = label + " " + std::to_string(int(score * 100)) + "%";
        int baseline;
        cv::Size text_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseline);
        cv::rectangle(result_img, 
                     cv::Point(rect.x, rect.y - text_size.height - 5),
                     cv::Point(rect.x + text_size.width, rect.y),
                     cv::Scalar(0, 255, 0), -1);
        cv::putText(result_img, text, cv::Point(rect.x, rect.y - 5),
                   cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
    }
    
    // 保存结果
    cv::imwrite(output_path, result_img);
    std::cout << "\n[Output] Result saved to: " << output_path << std::endl;

    // 反初始化
    yolo_graph->deinit();
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "  Detection Completed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return 0;
}
