#pragma once

#include <opencv2/core.hpp>
#include <string>
#include <vector>

namespace detector {

// The inference result for one frame, travelling on the inference ->
// persistence queue: the surviving detections (class ids + confidences), plus
// the annotated frame and its timestamp.
struct Detection {
    std::vector<int> classIds;
    std::vector<float> confidences;
    cv::Mat frame;
    std::string timestamp;

    Detection() = default;
    Detection(std::vector<int> ids, std::vector<float> conf, cv::Mat img, std::string ts)
        : classIds(std::move(ids)),
          confidences(std::move(conf)),
          frame(std::move(img)),
          timestamp(std::move(ts)) {}
};

} // namespace detector
