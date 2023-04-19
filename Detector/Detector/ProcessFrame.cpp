
#include "ProcessFrame.h"
#include "pch.h"
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "Detector.h"

using namespace cv;
using namespace std;

ProcessFrame::ProcessFrame()
{
	cfgReadeer = CfgLoader::instance();

	cfgReadeer->getCfgByName(Interval, "Interval");
	cfgReadeer->getCfgByName(DInterval, "DInterval");


}

//帧处理线程
void ProcessFrame::ThreadProcessFrame()
{
	Mat curFrame;
	string imageName;
	while (1) {
		Thread_mutex.lock();
		if (!Buffer.empty()) {
			FrameDO fd = Buffer.front();
			curFrame = fd.getFrame();
			imageName = fd.getTimeStamp();
			Buffer.pop_front();
		}
		else {
			curFrame = NULL;
		}
		Thread_mutex.unlock();
		if (!curFrame.empty()) {
			Process(curFrame, imageName);
		}
	}
}

void ProcessFrame::Process(Mat &frame, string imageName)
{
	
	double freq = getTickFrequency() / 1000;;

	//YOLOV3处理
	DetectorNet net;
	net.compute(frame);
	//检测到的标签及置信度
	vector<int> classIds=net.getClassIds();
	vector<float> confidences=net.getConfidences();
	vector<double> layersTimes=net.getLayersTimes();

	Thread_mutex1.lock();
	NetResultDO nrt(classIds,confidences,layersTimes,frame, imageName);
 	Buffer1.push_back(nrt);
	Thread_mutex1.unlock();
	
}






