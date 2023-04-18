#pragma once
#include "pch.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <deque>
#include <mutex>

#include "Configuration.h"
#include "db_Operator.h"
#include "ProcessFrame.h"
#include "Detector.h"
#include "DetectorNet.h"

class ProcessFrame {
	
public:

	//���ݿ����
	string DB_Name;
	string DB_User;
	string DB_Password;
	string DataSource;

    //Υ��ͼƬ����Ŀ¼
	string dirofdetectedframe;
	//��Ƶ��֡��
	float fps;
	//���μ��֮����ʱ��
	float Interval;
	//�ɹ���⵽Ŀ�����һ�μ����ʱ��
	float DInterval;



	string DirOfDetectedFrame;

	CfgLoader * cfgReadeer;

	ProcessFrame();

	void ThreadProcessFrame();

	void Process(Mat frame, string ImageName);

	int ProcessClass(vector<int>& classIds, int classid);
};