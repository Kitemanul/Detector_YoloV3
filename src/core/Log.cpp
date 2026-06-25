#include "detector/core/Log.h"

#include <chrono>
#include <ctime>
#include <iostream>
#include <mutex>

namespace detector {
namespace {

std::mutex g_logMutex;
LogLevel g_minLevel = LogLevel::Info;

const char* levelName(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO ";
        case LogLevel::Warn:  return "WARN ";
        case LogLevel::Error: return "ERROR";
    }
    return "?????";
}

std::string timestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm {};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buf);
}

} // namespace

void setLogLevel(LogLevel level) { g_minLevel = level; }

bool logEnabled(LogLevel level) { return level >= g_minLevel; }

LogMessage::LogMessage(LogLevel level, const char* /*file*/, int /*line*/)
    : level_(level) {}

LogMessage::~LogMessage() {
    std::lock_guard<std::mutex> lock(g_logMutex);
    std::clog << timestamp() << " [" << levelName(level_) << "] "
              << stream_.str() << std::endl;
}

} // namespace detector
