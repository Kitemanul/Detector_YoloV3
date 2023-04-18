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
	//��ǰ֡
	Mat curFrame;
	//ʹ�õ�������
	Net yolov3Net;
	Mat blob;
	//Ŀ��ʶ������ 00 01 10 11
	vector<string> classes;
	//�����ļ� 
	//voc.names
	//yolov3-voc.cfg
	//yolov3-voc_11400.weights
	//�����ļ�Ŀ¼��Ĭ����Ŀ��Ŀ¼
	string pro_dir = "";

	//��⵽�ı�ǩ�����Ŷ�
	vector<int> classIds;
	vector<float> confidences;
	vector<double> layersTimes;
	// ����������
	String modelConfiguration = pro_dir + "yolov3-voc.cfg";
	String modelWeights = pro_dir + "yolov3-voc_6000.weights";
	
	//����������������ļ�
	void loadConfig();
	//��ȡ������
	CfgLoader cfgReader;

	// ������ʼ��
	float confThreshold; // Confidence threshold
	float nmsThreshold = 0.4;  // Non-maximum suppression threshold
	int inpWidth = 416;  // Width of network's input image
	int inpHeight = 416; // Height of network's input image

	vector<Mat> outs;

	vector<String> getOutputsNames();

	//����
	void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat & frame);

public:
	//��ʼ��
	DetectorNet();
	//������
	void compute(Mat &frame);
	//����
	void postProcess();
	//��ȡ��ǩ
	vector<int> getClassIds();
	//��ȡ���Ŷ�
	vector<float> getConfidences();
	//��ȡ����ʱ��
	vector<double> getLayersTimes();




};

