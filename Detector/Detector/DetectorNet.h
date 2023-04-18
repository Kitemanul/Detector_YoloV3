#pragma once
#include <fstream>
#include <sstream>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace dnn;
using namespace cv;

class DetectorNet
{   
private:
	Net yolov3Net;
public:
	DetectorNet();
};

