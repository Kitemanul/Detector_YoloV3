#pragma once

#include <opencv2/core.hpp>
#include <string>

namespace detector {

// A single alarm to be persisted: the annotated frame plus its metadata.
struct AlarmRecord {
    cv::Mat frame;
    std::string imageName;  // file name to write the snapshot under
    std::string timestamp;  // human-readable capture time
    int level = 0;          // alarm severity (>0)
    float confidence = 0.f; // detector confidence for this alarm
};

// Abstract destination for detected violations. The pipeline depends only on
// this interface, so the storage backend (SQL Server, flat files, a message
// queue, ...) can be swapped without touching the core. Implementations are
// used from a single worker thread and need not be thread-safe themselves.
class ISink {
public:
    virtual ~ISink() = default;

    // Acquire resources (open a DB connection, create output dirs, ...).
    virtual void open() = 0;

    // Persist one alarm. Implementations should not throw across this boundary.
    virtual void record(const AlarmRecord& alarm) = 0;

    // Release resources.
    virtual void close() = 0;
};

} // namespace detector
