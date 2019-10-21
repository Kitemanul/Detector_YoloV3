#include "pch.h"
#include "db_Operator.h"


db_Operator::db_Operator(string dbType,string dbName,string userName,string pwd,string datasource)
{
	CoInitialize(NULL);
	m_pConn.CreateInstance(_uuidof(Connection));
	strCon = "Provider=SQLOLEDB.1;Persist Security Info=True;User ID="+userName+ ";Password=" +pwd+ ";Initial Catalog="+dbName+";Data Source="+datasource+";Integrated Security=SSPI;";
		//strCon = "DSN="+dbType+";server=localhost;database="+dbName;
	userName = dbName;
	pwd = pwd;
}
bool db_Operator::db_open()
{
	try
	{
		m_pConn->Open(strCon.c_str(), userName.c_str(), pwd.c_str(), adModeUnknown);
		return true;
	}
	catch (_com_error &e)
	{
		cout << e.Description() << endl;
		return false;
	}
}

_RecordsetPtr db_Operator::db_Query(_bstr_t sql)
{   rps=NULL;
	try
	{
		rps.CreateInstance("ADODB.Recordset");
        rps->Open(sql,
           _variant_t((IDispatch*)m_pConn,true),
           adOpenStatic,
           adLockOptimistic,
           adCmdText);
	}
	catch(_com_error &ce)
	{	printf("wrong:sql");
		_bstr_t error;
		error = ce.Description();
	}
	return rps;
}
bool db_Operator::db_Execute(_bstr_t sql)
{
	try
	{
		m_pConn->Execute(sql,NULL,adCmdText);
		return true;
	}
	catch(_com_error &ce)
	{
		_bstr_t error;
		error = ce.Description();
		cout << error << endl;
		return false;
	}
}
bool db_Operator::db_close()
{	  
	if(rps != NULL)
		rps->Close();
	m_pConn->Close();
	CoUninitialize();
	return true;
}

db_Operator::~db_Operator(void)
{
	CoUninitialize(); 
}

bool db_Operator::db_InsertRecord(float Acurecy,int Alarm_level,string Dir,string ImageName)
{
	db_open();	
	_bstr_t CreateSql = "if OBJECT_ID('DetectedRecord') is null create Table DetectedRecord(ID bigint primary key  identity(1, 1) not null,Time datetime null,DirDetectedSave nvarchar(50) null,Alarm_level nchar null,ImageName  nvarchar(50) null,Accurecy nvarchar(50))";
	db_Execute(CreateSql);

	//时间信息
	struct tm t;   //tm结构指针
	time_t now;  //声明time_t类型变量
	time(&now);      //获取系统日期和时间
	localtime_s(&t, &now);
	char s[30];
	sprintf_s(s, "%d-%02d-%02d %02d:%02d:%02d",
		t.tm_year + 1900,
		t.tm_mon + 1,
		t.tm_mday,
		t.tm_hour,
		t.tm_min,
		t.tm_sec);	
    //组成sql语句
	_bstr_t sql = "insert into dbo.DetectedRecord(Time,DirDetectedSave,Alarm_level,ImageName,Accurecy) values('" + _bstr_t(s) + "','" + _bstr_t(Dir.c_str()) + "','" + _bstr_t(Alarm_level)+"','" + _bstr_t(ImageName.c_str()) + "','" + _bstr_t(Acurecy) + "')";
	db_Execute(sql);
	db_close();
	return true;
}
