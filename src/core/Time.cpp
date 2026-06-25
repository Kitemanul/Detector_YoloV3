#include "detector/core/Time.h"

#include <chrono>
#include <ctime>

namespace detector {
namespace {

std::string format(const char* fmt) {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm {};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), fmt, &tm);
    return std::string(buf);
}

} // namespace

std::string nowForFilename() { return format("%Y-%m-%d_%H-%M-%S"); }

std::string nowForSql() { return format("%Y-%m-%d %H:%M:%S"); }

} // namespace detector
