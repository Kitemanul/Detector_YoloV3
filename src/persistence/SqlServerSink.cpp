#include "detector/persistence/SqlServerSink.h"

#if defined(_WIN32)

#include <opencv2/imgcodecs.hpp>

#include "detector/core/Log.h"

namespace detector {

SqlServerSink::SqlServerSink(std::string dbName, std::string user, std::string password,
                             std::string dataSource, std::string outputDir)
    : dbName_(std::move(dbName)),
      user_(std::move(user)),
      password_(std::move(password)),
      dataSource_(std::move(dataSource)),
      outputDir_(std::move(outputDir)) {}

void SqlServerSink::open() {
    conn_ = std::make_unique<AdoConnection>(dbName_, user_, password_, dataSource_);
    LOG(Info) << "SqlServerSink connected to " << dataSource_ << "/" << dbName_;
}

void SqlServerSink::record(const AlarmRecord& alarm) {
    const std::string path = outputDir_ + alarm.imageName;
    if (!cv::imwrite(path, alarm.frame)) {
        LOG(Error) << "Failed to write snapshot " << path;
        return;
    }
    conn_->insertRecord(alarm.confidence, alarm.level, outputDir_, alarm.imageName);
    LOG(Info) << "Recorded level-" << alarm.level << " alarm: " << alarm.imageName;
}

void SqlServerSink::close() {
    conn_.reset();
}

} // namespace detector

#endif // _WIN32
