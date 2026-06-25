#pragma once

// Windows / SQL Server only. Compiled when DETECTOR_WITH_SQLSERVER is defined.
#if defined(_WIN32)

#import "C:\\Program Files\\Common Files\\System\\ADO\\msado15.dll" \
    no_namespace rename("EOF", "adoEOF")

#include <string>

namespace detector {

// Low-level SQL Server access helper built on ADO (msado15). Owns a single
// connection and exposes connect / execute / insert primitives.
class AdoConnection {
public:
    AdoConnection(std::string dbName, std::string user, std::string password, std::string dataSource);
    ~AdoConnection();

    // Insert one violation record, creating the table on first use.
    bool insertRecord(float confidence, int alarmLevel, const std::string& dir, const std::string& imageName);

private:
    bool open();
    bool execute(_bstr_t sql);
    bool close();

    _ConnectionPtr conn_;
    std::string connectionString_;
    std::string user_;
    std::string password_;
};

} // namespace detector

#endif // _WIN32
