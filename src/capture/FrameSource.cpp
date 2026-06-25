#include "detector/capture/FrameSource.h"

#include <fstream>
#include <stdexcept>

namespace detector {

FrameSource::FrameSource(SourceType type, const std::string& uri) : type_(type) {
    if (type_ == SourceType::Image || type_ == SourceType::Video) {
        // Fail fast if the file does not exist, rather than letting
        // VideoCapture silently fall back to other behaviour.
        std::ifstream probe(uri);
        if (!probe) {
            throw std::runtime_error("Input file not found: " + uri);
        }
    }
    if (!cap_.open(uri)) {
        throw std::runtime_error("Could not open input source: " + uri);
    }
}

bool FrameSource::read(cv::Mat& frame) {
    return cap_.read(frame);
}

void FrameSource::release() {
    cap_.release();
}

} // namespace detector
