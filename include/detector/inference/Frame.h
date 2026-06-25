#pragma once

#include <opencv2/core.hpp>
#include <string>

namespace detector {

// A single captured frame travelling on the capture -> inference queue,
// together with the timestamp at which it was sampled.
struct Frame {
    cv::Mat image;
    std::string timestamp;

    Frame() = default;
    Frame(cv::Mat img, std::string ts)
        : image(std::move(img)), timestamp(std::move(ts)) {}
};

} // namespace detector
