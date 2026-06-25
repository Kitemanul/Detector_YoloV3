#include "ProcessFrame.h"
#include "pch.h"
#include <chrono>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "Detector.h"

using namespace cv;
using namespace std;

ProcessFrame::ProcessFrame()
{
	cfgReader = CfgLoader::instance();

	cfgReader->getCfgByName(Interval, "Interval");
	cfgReader->getCfgByName(DInterval, "DInterval");
}

// Inference worker loop.
void ProcessFrame::ThreadProcessFrame()
{
	Mat curFrame;
	string imageName;
	while (true) {
		bool hasFrame = false;
		{
			lock_guard<mutex> lock(Thread_mutex);
			if (!Buffer.empty()) {
				FrameDO fd = Buffer.front();
				curFrame = fd.getFrame();
				imageName = fd.getTimeStamp();
				Buffer.pop_front();
				hasFrame = true;
			}
		}
		if (hasFrame && !curFrame.empty()) {
			Process(curFrame, imageName);
		}
		else {
			// Nothing to do; yield instead of busy-spinning the CPU.
			this_thread::sleep_for(chrono::milliseconds(5));
		}
	}
}

void ProcessFrame::Process(Mat &frame, string imageName)
{
	// YOLOv3 inference.
	DetectorNet net;
	net.compute(frame);
	// Detected labels, confidences and per-layer timings.
	vector<int> classIds = net.getClassIds();
	vector<float> confidences = net.getConfidences();
	vector<double> layersTimes = net.getLayersTimes();

	{
		lock_guard<mutex> lock(Thread_mutex1);
		NetResultDO nrt(classIds, confidences, layersTimes, frame, imageName);
		Buffer1.push_back(nrt);
	}
}
