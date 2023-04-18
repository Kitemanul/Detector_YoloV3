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
	//ʹ�õ�������
	Net yolov3Net;
	//Ŀ��ʶ������ 00 01 10 11
	vector<string> classes;
public:
	DetectorNet();
};

