#pragma once

#include <string>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include "Configuration.h"
#include "db_Operator.h"
#include <mutex>
#include <deque>
using namespace std;
using namespace cv;
using namespace dnn;

extern float confThreshold;
extern float confThreshold; // Confidence threshold
extern float nmsThreshold;  // Non-maximum suppression threshold
extern int inpWidth;  // Width of network's input image
extern int inpHeight;
extern vector<string> classes;
extern string pro_dir;
extern CfgLoader *Configuration;
extern string DB_Name;
extern string DB_User;
extern string DB_Password;
extern string DataSource;
extern db_Operator *dbo;
extern mutex Thread_mutex;
extern string DirOfDetectedFrame;
//视频流帧率
extern float FPS;
//两次检测之间间隔时间
extern float Interval;
//成功检测到目标后，下一次检测间隔时间
extern float DInterval;
extern float slot;

extern deque<Mat> Buffer;
extern deque<string> Imagename;

class ProcessFrame {
public:
	
	void postprocess(Mat& frame, const vector<Mat>& outs, vector<int>& classIds, vector<float>& confidences);
	
	void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame);

	vector<String> getOutputsNames(const Net& net);

	void ThreadProcessFrame();

	void Process(Mat frame, Net net, string ImageName);

	Net LoadNetCfg();

	int ProcessClass(vector<int>& classIds, int classid);
};