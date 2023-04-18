
#include "ProcessFrame.h"
#include "pch.h"


//֡�����߳�
void ProcessFrame::ThreadProcessFrame()
{
	Mat curFrame;
	int flag = 0;
	while (1){
		Thread_mutex.lock();
		if (!Buffer.empty()){
			flag = 0;
			curFrame = Buffer.front();
			ImageName = Imagename.front();
			Buffer.pop_front();
			Imagename.pop_front();
		}else{
			curFrame = NULL;
			flag++;
		}
		Thread_mutex.unlock();
		if (!curFrame.empty()){
			Process(curFrame,ImageName);
		}
		if (flag == 10){
			break;
		}
	}
}

void ProcessFrame::Process(Mat frame, string ImageName)
{
	////�½����ݿ��������
	db_Operator dbo("sql server", DB_Name, DB_User, DB_Password, DataSource);


	double freq = getTickFrequency() / 1000;;
	double t;

	//YOLOV3����
	DetectorNet net;
	net.compute(frame);

	//��⵽�ı�ǩ�����Ŷ�
	vector<int> classIds=net.getClassIds();
	vector<float> confidences=net.getConfidences();
	vector<double> layersTimes=net.getLayersTimes();

	for (int i = 0; i < net.getClassIds.size(); i++)
	{
		switch (ProcessClass(classIds, i))
		{
		case 1:
			imwrite(DirOfDetectedFrame + ImageName + "Warning1" + "_" + to_string(i) + ".jpg", frame);
			dbo.db_InsertRecord(confidences[i], 1, DirOfDetectedFrame, ImageName + "Warning1" + "_" + to_string(i) + ".jpg");
			slot = DInterval;
			cout << "Save Detected Frame!" << endl;
			break;
		case 2:
			imwrite(DirOfDetectedFrame + ImageName + "Warning2" + "_" + to_string(i) + ".jpg", frame);
			dbo.db_InsertRecord(confidences[i], 2, DirOfDetectedFrame, ImageName + "Warning2" + "_" + to_string(i) + ".jpg");
			slot = DInterval;
			cout << "Save Detected Frame!" << endl;
			break;
		case 3:
			imwrite(DirOfDetectedFrame + ImageName + "Warning3" + "_" + to_string(i) + ".jpg", frame);
			dbo.db_InsertRecord(confidences[i], 3, DirOfDetectedFrame, ImageName + "Warning3" + "_" + to_string(i) + ".jpg");
			slot = DInterval;
			cout << "Save Detected Frame!" << endl;
			break;
		case 0:break;
		}
	}
}

//�����⵽Υ���Ŀ�꣬����true
int ProcessFrame::ProcessClass(vector<int>& classIds, int classid)
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




