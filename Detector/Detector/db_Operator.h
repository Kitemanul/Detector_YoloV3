#pragma once
#import "C:\\Program Files\\Common Files\\System\\ADO\\msado15.dll" \
no_namespace rename("EOF", "adoEOF")
#include <string>
#include <iostream>
#include <time.h>
using namespace std;

// Low-level SQL Server access helper built on ADO (msado15). Owns a single
// connection and exposes open / query / execute / close primitives plus the
// domain-specific insert used to record detected violations.
class db_Operator
{
public:
	db_Operator(string dbType, string dbName, string userName, string pwd, string datasource);
	~db_Operator(void);
	// Current time formatted for file names ("YYYY-MM-DD_HH-MM-SS").
	static string get_CurrentTime_s();
	// Current time formatted for SQL datetime ("YYYY-MM-DD HH:MM:SS").
	static string get_CurrentTime();
	// Insert one violation record (creating the table on first use).
	bool db_InsertRecord(float Acurecy, int Alarm_level, string Dir, string ImageName);
	_RecordsetPtr db_Query(_bstr_t sql);
	bool db_Execute(_bstr_t sql);
	bool db_close();
	bool db_open();
public:
	_ConnectionPtr m_pConn;
	_RecordsetPtr rps;
private:
	string strCon;
	string userName;
	string pwd;
};
