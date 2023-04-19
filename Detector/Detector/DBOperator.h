#pragma once
#include "db_Operator.h"
#include <vector>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "Configuration.h"
#include "Detector.h"
#include "NetResultDO.h"

using namespace std;
using namespace cv;

class DBOperator
{
private:
	//数据库参数
	string DB_Name;
	string DB_User;
	string DB_Password;
	string DataSource;
	string DirOfDetectedFrame;
	
public:
	DBOperator();

	CfgLoader * cfgReadeer;

	int ProcessClass(vector<int>& classIds, int classid);

	void threadInsertAlertInf();

};

