#pragma once
#include <opencv2/imgproc.hpp>
using namespace std;
using namespace cv;
class FrameDO
{	
private:
	//Í¼ÏñÖ¡
	Mat frame;

	//Ê±¼ä´Á
	string timestamp;
public :

	FrameDO(Mat frame,string ts);

	Mat getFrame();

	void setFrame(Mat &frame);

	string getTimeStamp();

	void setTimeStamp(string ts);
};

