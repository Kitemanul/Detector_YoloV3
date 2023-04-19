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
    //违规图片保存目录
	string dirofdetectedframe;
	//视频流帧率
	float fps;
	//两次检测之间间隔时间
	float Interval;
	//成功检测到目标后，下一次检测间隔时间
	float DInterval;

	CfgLoader * cfgReadeer;

	ProcessFrame();

	void ThreadProcessFrame();

	void Process(Mat &frame, string ImageName);


};