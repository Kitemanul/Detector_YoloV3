#pragma once

#include <sstream>
#include <string>

namespace detector {

enum class LogLevel { Debug, Info, Warn, Error };

// Minimal thread-safe, leveled logger. Each record is timestamped and written
// to stderr under a single mutex, so messages from the pipeline's worker
// threads never interleave mid-line.
//
// Usage:  LOG(Info) << "processed " << n << " frames";
class LogMessage {
public:
    LogMessage(LogLevel level, const char* file, int line);
    ~LogMessage();

    template <typename T>
    LogMessage& operator<<(const T& value) {
        stream_ << value;
        return *this;
    }

private:
    LogLevel level_;
    std::ostringstream stream_;
};

// The minimum level that will actually be emitted (default: Info).
void setLogLevel(LogLevel level);
bool logEnabled(LogLevel level);

} // namespace detector

#define LOG(level)                                                            \
    if (!::detector::logEnabled(::detector::LogLevel::level)) {                \
    } else                                                                     \
        ::detector::LogMessage(::detector::LogLevel::level, __FILE__, __LINE__)
