#pragma once
#include <opencv2/imgproc.hpp>
using namespace std;
using namespace cv;

// Data object carried on the capture -> inference queue: one sampled frame and
// the timestamp at which it was captured.
class FrameDO
{
private:
	// Image frame.
	Mat frame;
	// Capture timestamp.
	string timestamp;
public:

	FrameDO(Mat frame, string ts);

	Mat getFrame();

	void setFrame(Mat &frame);

	string getTimeStamp();

	void setTimeStamp(string ts);
};
