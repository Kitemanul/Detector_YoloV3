#pragma once

#if defined(_WIN32)

#include <memory>
#include <string>

#include "detector/persistence/AdoConnection.h"
#include "detector/persistence/ISink.h"

namespace detector {

// SQL Server sink: writes the violation snapshot to disk and inserts a row into
// the DetectedRecord table through an ADO connection. Windows only.
class SqlServerSink : public ISink {
public:
    SqlServerSink(std::string dbName, std::string user, std::string password,
                  std::string dataSource, std::string outputDir);

    void open() override;
    void record(const AlarmRecord& alarm) override;
    void close() override;

private:
    std::string dbName_;
    std::string user_;
    std::string password_;
    std::string dataSource_;
    std::string outputDir_;
    std::unique_ptr<AdoConnection> conn_;
};

} // namespace detector

#endif // _WIN32
