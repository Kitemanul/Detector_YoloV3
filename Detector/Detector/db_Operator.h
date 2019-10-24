#pragma once
#import "C:\\Program Files\\Common Files\\System\\ADO\\msado15.dll" \
no_namespace rename("EOF", "adoEOF")
#include <string>
#include <iostream>
#include <time.h>
using namespace std;
class db_Operator
{
public:
	db_Operator(string dbType, string dbName, string userName, string pwd, string datasource);
	~db_Operator(void);
	static string get_CurrentTime_s();
	static string get_CurrentTime();
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

