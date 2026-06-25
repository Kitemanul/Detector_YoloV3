#include "detector/persistence/AdoConnection.h"

#if defined(_WIN32)

#include <iostream>

#include "detector/core/Time.h"

namespace detector {

AdoConnection::AdoConnection(std::string dbName, std::string user, std::string password, std::string dataSource)
    : user_(std::move(user)), password_(std::move(password)) {
    CoInitialize(NULL);
    conn_.CreateInstance(_uuidof(Connection));
    connectionString_ = "Provider=SQLOLEDB.1;Persist Security Info=False;User ID=" + user_ +
                        ";Password=" + password_ + ";Initial Catalog=" + dbName +
                        ";Data Source=" + dataSource + ";";
}

AdoConnection::~AdoConnection() {
    CoUninitialize();
}

bool AdoConnection::open() {
    try {
        conn_->Open(connectionString_.c_str(), user_.c_str(), password_.c_str(), adModeUnknown);
        return true;
    } catch (_com_error& e) {
        std::cout << "Database error: " << (const char*)e.Description() << std::endl;
        return false;
    }
}

bool AdoConnection::execute(_bstr_t sql) {
    try {
        conn_->Execute(sql, NULL, adCmdText);
        return true;
    } catch (_com_error& e) {
        std::cout << "Failed to execute SQL: " << (const char*)e.Description() << std::endl;
        return false;
    }
}

bool AdoConnection::close() {
    conn_->Close();
    return true;
}

bool AdoConnection::insertRecord(float confidence, int alarmLevel, const std::string& dir, const std::string& imageName) {
    if (!open()) {
        return false;
    }
    _bstr_t createSql = "if OBJECT_ID('DetectedRecord') is null create Table DetectedRecord("
                        "ID bigint primary key identity(1, 1) not null,Time datetime null,"
                        "DirDetectedSave nvarchar(50) null,Alarm_level nchar null,"
                        "ImageName nvarchar(50) null,Accurecy nvarchar(50))";
    execute(createSql);

    _bstr_t sql = "insert into dbo.DetectedRecord(Time,DirDetectedSave,Alarm_level,ImageName,Accurecy) values('" +
                  _bstr_t(nowForSql().c_str()) + "','" + _bstr_t(dir.c_str()) + "','" +
                  _bstr_t(alarmLevel) + "','" + _bstr_t(imageName.c_str()) + "','" +
                  _bstr_t(confidence) + "')";
    execute(sql);
    close();
    return true;
}

} // namespace detector

#endif // _WIN32
