
#include "ProcessFrame.h"
#include "pch.h"
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "Detector.h"

using namespace cv;
using namespace std;

ProcessFrame::ProcessFrame()
{
	cfgReadeer = CfgLoader::instance();


	cfgReadeer->getCfgByName(DirOfDetectedFrame, "DetectedFrameDir");//get违规图片保存目录
	cfgReadeer->getCfgByName(Interval, "Interval");
	cfgReadeer->getCfgByName(DInterval, "DInterval");
	//load database cfg
	cfgReadeer->getCfgByName(DataSource, "DataSource");
	cfgReadeer->getCfgByName(DB_Name, "DB_Name");
	cfgReadeer->getCfgByName(DB_User, "DB_User");
	cfgReadeer->getCfgByName(DB_Password, "DB_Password");

}

//帧处理线程
void ProcessFrame::ThreadProcessFrame()
{
	Mat curFrame;
	int flag = 0;
	string imageName;
	while (1) {
		Thread_mutex.lock();
		if (!Buffer.empty()) {
			flag = 0;
			curFrame = Buffer.front();
			imageName = ImageName.front();
			Buffer.pop_front();
			ImageName.pop_front();
		}
		else {
			curFrame = NULL;
			flag++;
		}
		Thread_mutex.unlock();
		if (!curFrame.empty()) {
			Process(curFrame, imageName);
		}
		if (flag == 10) {
			break;
		}
	}
}

void ProcessFrame::Process(Mat frame, string imageName)
{
	////新建数据库操作对象
	db_Operator dbo("sql server", DB_Name, DB_User, DB_Password, DataSource);


	double freq = getTickFrequency() / 1000;;
	double t;

	//YOLOV3处理
	DetectorNet net;
	net.compute(frame);

	//检测到的标签及置信度
	vector<int> classIds=net.getClassIds();
	vector<float> confidences=net.getConfidences();
	vector<double> layersTimes=net.getLayersTimes();

	for (int i = 0; i < classIds.size(); i++)
	{
		switch (ProcessClass(classIds, i))
		{
		case 1:
			imwrite(DirOfDetectedFrame + imageName + "Warning1" + "_" + to_string(i) + ".jpg", frame);
			dbo.db_InsertRecord(confidences[i], 1, DirOfDetectedFrame, imageName + "Warning1" + "_" + to_string(i) + ".jpg");
			slot = DInterval;
			cout << "Save Detected Frame!" << endl;
			break;
		case 2:
			imwrite(DirOfDetectedFrame + imageName + "Warning2" + "_" + to_string(i) + ".jpg", frame);
			dbo.db_InsertRecord(confidences[i], 2, DirOfDetectedFrame, imageName + "Warning2" + "_" + to_string(i) + ".jpg");
			slot = DInterval;
			cout << "Save Detected Frame!" << endl;
			break;
		case 3:
			imwrite(DirOfDetectedFrame + imageName + "Warning3" + "_" + to_string(i) + ".jpg", frame);
			dbo.db_InsertRecord(confidences[i], 3, DirOfDetectedFrame, imageName + "Warning3" + "_" + to_string(i) + ".jpg");
			slot = DInterval;
			cout << "Save Detected Frame!" << endl;
			break;
		case 0:break;
		}
	}
}

//如果检测到违规的目标，返回true
int ProcessFrame::ProcessClass(vector<int>& classIds, int classid)
{

	switch (classIds[classid])
	{
	case 0:return 1;  //一级警告

	case 1:return 2;  //二级警告

	case 2:return 3;  //三级警告

	default:return 0; //正常			     
	}

	return false;
}




