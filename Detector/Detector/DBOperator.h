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

// Database worker. Consumes inference results, classifies each detection into an
// alarm level, writes violation snapshots to disk and records them in SQL Server.
class DBOperator
{
private:
	// Database connection settings.
	string DB_Name;
	string DB_User;
	string DB_Password;
	string DataSource;
	// Output directory for violation snapshots.
	string DirOfDetectedFrame;

public:
	DBOperator();

	CfgLoader * cfgReader;

	// Map a detected class id to an alarm level (0 = compliant, >0 = violation).
	int ProcessClass(vector<int>& classIds, int classid);

	// Worker loop: drains the inference-result queue and persists violations.
	void threadInsertAlertInf();
};
