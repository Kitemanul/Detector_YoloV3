#pragma once
#include <opencv2/imgproc.hpp>
using namespace std;
using namespace cv;
class FrameDO
{	
private:
	//ͼ��֡
	Mat frame;

	//ʱ���
	string timestamp;
public :

	FrameDO(Mat frame,string ts);

	Mat getFrame();

	void setFrame(Mat &frame);

	string getTimeStamp();

	void setTimeStamp(string ts);
};

