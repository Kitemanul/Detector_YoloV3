#pragma once
#include <opencv2/imgproc.hpp>
#include <vector>

using namespace std;
using namespace cv;

// Data object carried on the inference -> database queue: the detection result
// for one frame together with the frame itself and its timestamp.
class NetResultDO
{
private:
	// Detected labels, confidences and per-layer timings.
	vector<int> classIds;
	vector<float> confidences;
	vector<double> layersTimes;
	Mat frame;
	string timestamp;

public:

	NetResultDO(vector<int> classIds, vector<float> confidences, vector<double> layersTimes, Mat frame, string ts);
	vector<int> getClssIds();
	vector<float> getConfidences();
	vector<double> getLayersTimes();
	Mat getFrame();
	string getTs();

	void setClassIds(vector<int> classIds);
	void setConfidences(vector<float> confidences);
	void setLayersTimes(vector<double> layersTimes);
};
