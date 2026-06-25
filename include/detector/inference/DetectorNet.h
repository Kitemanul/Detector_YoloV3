#pragma once

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>

namespace detector {

// Wrapper around the OpenCV DNN module that loads a YOLOv3 (Darknet) network
// once and runs forward inference on individual frames.
//
// A single instance is reused for the whole run (the network is loaded from
// disk exactly once); compute() resets the per-frame result vectors on every
// call.
class DetectorNet {
public:
    DetectorNet();

    // Run a forward pass on `frame` (which is annotated in place with the
    // detected boxes). Results are then available through the accessors below.
    void compute(cv::Mat& frame);

    const std::vector<int>& classIds() const { return classIds_; }
    const std::vector<float>& confidences() const { return confidences_; }
    const std::vector<std::string>& classNames() const { return classes_; }

private:
    void loadConfig();
    std::vector<cv::String> outputNames();
    void postProcess();
    void drawPred(int classId, float conf, int left, int top, int right, int bottom, cv::Mat& frame);

    cv::Mat curFrame_;
    cv::dnn::Net net_;
    cv::Mat blob_;
    std::vector<std::string> classes_;

    // Directory holding the model files; empty means the working directory.
    std::string modelDir_;
    cv::String modelConfiguration_;
    cv::String modelWeights_;

    // Per-frame results (cleared at the start of every compute()).
    std::vector<int> classIds_;
    std::vector<float> confidences_;
    std::vector<cv::Mat> outs_;

    // Inference hyper-parameters.
    float confThreshold_ = 0.5f;
    float nmsThreshold_ = 0.4f;
    int inpWidth_ = 416;
    int inpHeight_ = 416;
};

} // namespace detector
