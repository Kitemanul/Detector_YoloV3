#include "pch.h"
#include <chrono>
#include <thread>
#include "DBOperator.h"

DBOperator::DBOperator()
{
	cfgReader = CfgLoader::instance();
	// Load database configuration.
	cfgReader->getCfgByName(DataSource, "DataSource");
	cfgReader->getCfgByName(DB_Name, "DB_Name");
	cfgReader->getCfgByName(DB_User, "DB_User");
	cfgReader->getCfgByName(DB_Password, "DB_Password");
	// Output directory for violation snapshots.
	cfgReader->getCfgByName(DirOfDetectedFrame, "DetectedFrameDir");
}

void DBOperator::threadInsertAlertInf()
{
	// Create the database connection helper.
	db_Operator dbo("sql server", DB_Name, DB_User, DB_Password, DataSource);

	Mat frame;
	String imageName;
	vector<float> confidences;
	vector<int> classIds;
	while (true) {
		bool hasResult = false;
		{
			lock_guard<mutex> lock(Thread_mutex1);
			if (!Buffer1.empty()) {
				NetResultDO nrt = Buffer1.front();
				frame = nrt.getFrame();
				imageName = nrt.getTs();
				confidences = nrt.getConfidences();
				classIds = nrt.getClssIds();
				Buffer1.pop_front();
				hasResult = true;
			}
		}
		if (!hasResult) {
			// Nothing to do; yield instead of busy-spinning the CPU.
			this_thread::sleep_for(chrono::milliseconds(5));
			continue;
		}
		for (size_t i = 0; i < classIds.size(); i++)
		{
			int level = ProcessClass(classIds, (int)i);
			if (level == 0) {
				continue; // compliant; nothing to record
			}
			string suffix = "Warning" + to_string(level) + "_" + to_string(i) + ".jpg";
			imwrite(DirOfDetectedFrame + imageName + suffix, frame);
			dbo.db_InsertRecord(confidences[i], level, DirOfDetectedFrame, imageName + suffix);
			cout << "Save Detected Frame!" << endl;
		}
	}
}

// Map a detected class id to an alarm level.
//   class 0 -> level 1 (most severe)
//   class 1 -> level 2
//   class 2 -> level 3
//   otherwise -> 0 (compliant, no alarm)
int DBOperator::ProcessClass(vector<int>& classIds, int classid)
{
	switch (classIds[classid])
	{
	case 0: return 1;
	case 1: return 2;
	case 2: return 3;
	default: return 0;
	}
}
