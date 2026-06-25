#pragma once

#include <fstream>
#include <string>

#include "detector/persistence/ISink.h"

namespace detector {

// Portable sink: writes each violation snapshot as a JPEG into an output
// directory and appends a row to a CSV log. Works on any platform with OpenCV
// and is the default backend when SQL Server support is not compiled in.
class FileSink : public ISink {
public:
    explicit FileSink(std::string outputDir);

    void open() override;
    void record(const AlarmRecord& alarm) override;
    void close() override;

private:
    std::string outputDir_;
    std::ofstream csv_;
};

} // namespace detector
