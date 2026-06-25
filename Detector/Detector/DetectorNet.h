#pragma once
#include <fstream>
#include <sstream>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "Configuration.h"

using namespace std;
using namespace cv;
using namespace dnn;

// Thin wrapper around the OpenCV DNN module that loads a YOLOv3 (Darknet)
// network and runs forward inference on a single frame.
class DetectorNet
{
private:
	// Frame currently being processed.
	Mat curFrame;
	// The YOLOv3 network.
	Net yolov3Net;
	Mat blob;
	// Object class labels, e.g. 00 / 01 / 10 / 11.
	vector<string> classes;
	// Directory holding the model files (voc.names, yolov3-voc.cfg, *.weights).
	// Empty string means the executable's working directory.
	string pro_dir = "";

	// Detected class ids and their confidence scores for the current frame.
	vector<int> classIds;
	vector<float> confidences;
	vector<double> layersTimes;
	// Network definition files.
	String modelConfiguration;
	String modelWeights;

	// Configuration reader (singleton).
	CfgLoader* cfgReader;

	// Inference hyper-parameters.
	float confThreshold;        // confidence threshold
	float nmsThreshold = 0.4;   // non-maximum suppression threshold
	int inpWidth = 416;         // width of the network's input image
	int inpHeight = 416;        // height of the network's input image

	vector<Mat> outs;

	vector<String> getOutputsNames();

	// Draw a single predicted bounding box onto the frame.
	void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat & frame);

public:

	void loadConfig();
	// Constructor: loads config and builds the network.
	DetectorNet();
	// Run a forward pass on the given frame.
	void compute(Mat &frame);
	// Decode raw network output into bounding boxes and labels.
	void postProcess();
	// Accessors for the latest inference result.
	vector<int> getClassIds();
	vector<float> getConfidences();
	vector<double> getLayersTimes();
};
