#include "detector/persistence/FileSink.h"

#include <opencv2/imgcodecs.hpp>

#include "detector/core/Log.h"

namespace detector {

FileSink::FileSink(std::string outputDir) : outputDir_(std::move(outputDir)) {}

void FileSink::open() {
    const std::string csvPath = outputDir_ + "detections.csv";
    // Detect whether the log already exists so the header is written only once.
    bool isNew = !std::ifstream(csvPath).good();
    // Append so that records from successive runs are preserved.
    csv_.open(csvPath, std::ios::out | std::ios::app);
    if (isNew && csv_.is_open()) {
        csv_ << "timestamp,level,confidence,image\n";
    }
    LOG(Info) << "FileSink writing to " << outputDir_;
}

void FileSink::record(const AlarmRecord& alarm) {
    const std::string path = outputDir_ + alarm.imageName;
    if (!cv::imwrite(path, alarm.frame)) {
        LOG(Error) << "Failed to write snapshot " << path;
        return;
    }
    if (csv_.is_open()) {
        csv_ << alarm.timestamp << ',' << alarm.level << ','
             << alarm.confidence << ',' << alarm.imageName << '\n';
        csv_.flush();
    }
    LOG(Info) << "Recorded level-" << alarm.level << " alarm: " << alarm.imageName;
}

void FileSink::close() {
    if (csv_.is_open()) {
        csv_.close();
    }
}

} // namespace detector
