#include "pch.h"
#include "DBOperator.h"

DBOperator::DBOperator()
{
	cfgReadeer = CfgLoader::instance();
	//load database cfg
	cfgReadeer->getCfgByName(DataSource, "DataSource");
	cfgReadeer->getCfgByName(DB_Name, "DB_Name");
	cfgReadeer->getCfgByName(DB_User, "DB_User");
	cfgReadeer->getCfgByName(DB_Password, "DB_Password");
	cfgReadeer->getCfgByName(DirOfDetectedFrame, "DetectedFrameDir");//getΥ��ͼƬ����Ŀ¼
	
}

void DBOperator::threadInsertAlertInf()
{
	////�½����ݿ��������
	db_Operator dbo("sql server", DB_Name, DB_User, DB_Password, DataSource);

	Mat frame;
	String imageName;
	vector<float> confidences;
	vector<int> classIds;
	bool flag = false;
	while (1) {
		Thread_mutex1.lock();
		if (!Buffer1.empty()) {
			flag = true;
			NetResultDO nrt = Buffer1.front();
			frame = nrt.getFrame();
			imageName = nrt.getTs();
			confidences = nrt.getConfidences();
			classIds = nrt.getClssIds();
			Buffer1.pop_front();
		}
		else {
			flag = false;
		}
		Thread_mutex1.unlock();
		if (!flag) {
			continue;
		}
		for (int i = 0; i < classIds.size(); i++)
		{
			switch (ProcessClass(classIds, i))
			{
			case 1:
				imwrite(DirOfDetectedFrame + imageName + "Warning1" + "_" + to_string(i) + ".jpg", frame);
				dbo.db_InsertRecord(confidences[i], 1, DirOfDetectedFrame, imageName + "Warning1" + "_" + to_string(i) + ".jpg");
				cout << "Save Detected Frame!" << endl;
				break;
			case 2:
				imwrite(DirOfDetectedFrame + imageName + "Warning2" + "_" + to_string(i) + ".jpg", frame);
				dbo.db_InsertRecord(confidences[i], 2, DirOfDetectedFrame, imageName + "Warning2" + "_" + to_string(i) + ".jpg");
				cout << "Save Detected Frame!" << endl;
				break;
			case 3:
				imwrite(DirOfDetectedFrame + imageName + "Warning3" + "_" + to_string(i) + ".jpg", frame);
				dbo.db_InsertRecord(confidences[i], 3, DirOfDetectedFrame, imageName + "Warning3" + "_" + to_string(i) + ".jpg");
				cout << "Save Detected Frame!" << endl;
				break;
			case 0:break;
			}
		}
	}	
}

//�����⵽Υ���Ŀ�꣬����true
int DBOperator::ProcessClass(vector<int>& classIds, int classid)
{

	switch (classIds[classid])
	{
	case 0:return 1;  //һ������

	case 1:return 2;  //��������

	case 2:return 3;  //��������

	default:return 0; //����			     
	}

	return false;
}
