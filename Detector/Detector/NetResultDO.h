#pragma once
#include <opencv2/imgproc.hpp>
#include <vector>

using namespace std;
using namespace cv;

class NetResultDO
{
private:
	//检测到的标签及置信度
	vector<int> classIds ;
	vector<float> confidences ;
	vector<double> layersTimes ;
	Mat frame;
	string timestamp;

public :

	NetResultDO(vector<int> classIds, vector<float> confidences, vector<double> layersTimes,Mat frame,string ts);
	vector<int> getClssIds();
	vector<float> getConfidences();
	vector<double> getLayersTimes();
	Mat getFrame();
	string getTs();

	void setClassIds(vector<int> classIds);
	void setConfidences(vector<float> confidences);
	void setLayersTimes(vector<double> layersTimes);

};

