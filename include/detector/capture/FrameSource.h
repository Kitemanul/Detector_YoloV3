#pragma once

#include <opencv2/videoio.hpp>
#include <string>

namespace detector {

// Describes where frames come from.
enum class SourceType { Image, Video, Rtsp };

// Thin wrapper around cv::VideoCapture that knows how to open an image, a video
// file, or an RTSP/webcam stream, and reports whether the source is a live
// stream (which influences the queue backpressure policy).
class FrameSource {
public:
    // Open the given source. Throws std::runtime_error on failure.
    FrameSource(SourceType type, const std::string& uri);

    // Read the next frame. Returns false at end-of-stream / on read failure.
    bool read(cv::Mat& frame);

    // True for RTSP/webcam inputs, where stale frames should be dropped.
    bool isLive() const { return type_ == SourceType::Rtsp; }

    void release();

private:
    SourceType type_;
    cv::VideoCapture cap_;
};

} // namespace detector
