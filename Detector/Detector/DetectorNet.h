#pragma once
#include <fstream>
#include <sstream>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "Configuration.h"

using namespace std;
using namespace dnn;
using namespace cv;

class DetectorNet
{   
private:
	//当前帧
	Mat curFrame;
	//使用的神经网络
	Net yolov3Net;
	Mat blob;
	//目标识别的类别 00 01 10 11
	vector<string> classes;
	//配置文件 
	//voc.names
	//yolov3-voc.cfg
	//yolov3-voc_11400.weights
	//配置文件目录，默认项目根目录
	string pro_dir = "";

	//检测到的标签及置信度
	vector<int> classIds;
	vector<float> confidences;
	vector<double> layersTimes;
	// 神经网络配置
	String modelConfiguration = pro_dir + "yolov3-voc.cfg";
	String modelWeights = pro_dir + "yolov3-voc_6000.weights";
	
	//载入神经网络的配置文件
	void loadConfig();
	//读取配置类
	CfgLoader cfgReader;

	// 参数初始化
	float confThreshold; // Confidence threshold
	float nmsThreshold = 0.4;  // Non-maximum suppression threshold
	int inpWidth = 416;  // Width of network's input image
	int inpHeight = 416; // Height of network's input image

	vector<Mat> outs;

	vector<String> getOutputsNames();

	//画框
	void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat & frame);

public:
	//初始化
	DetectorNet();
	//计算结果
	void compute(Mat &frame);
	//后处理
	void postProcess();
	//获取标签
	vector<int> getClassIds();
	//获取置信度
	vector<float> getConfidences();
	//获取处理时间
	vector<double> getLayersTimes();




};

