#include "pch.h"
#include "DetectorNet.h"


DetectorNet::DetectorNet()
{
	loadConfig();
	// Load the network
	yolov3Net = dnn::readNetFromDarknet(modelConfiguration, modelWeights);
	yolov3Net.setPreferableBackend(dnn::DNN_BACKEND_OPENCV);
	yolov3Net.setPreferableTarget(dnn::DNN_TARGET_CPU);
}

void DetectorNet::loadConfig()
{
	//Load Cfg
	cfgReader.init("Configuration.cfg");
	cfgReader.getCfgByName(confThreshold, "Threshold");
	cfgReader.getCfgByName(pro_dir, "NNCfg_Dir");
	// Load names of classes 00 01 10 11
	string classesFile = pro_dir + "voc.names";
	ifstream ifs(classesFile.c_str());
	string line;
	while (getline(ifs, line)) classes.push_back(line);

}

void DetectorNet::process(Mat &frame)
{	
	curFrame = frame;
	// Create a 4D blob from a frame.
	dnn::blobFromImage(frame, blob, 1 / 255.0, cvSize(inpWidth, inpHeight), Scalar(0, 0, 0), true, false);
	//Sets the input to the network
	yolov3Net.setInput(blob);
	// Runs the forward pass to get output of the output layers	
	yolov3Net.forward(outs, getOutputsNames());
}

void DetectorNet::postProcess()
{
}

// Get the names of the output layers
vector<String> DetectorNet::getOutputsNames()
{	
	static vector<String> names;
	if (names.empty())
	{
		//Get the indices of the output layers, i.e. the layers with unconnected outputs
		vector<int> outLayers = yolov3Net.getUnconnectedOutLayers();

		//get the names of all the layers in the network
		vector<String> layersNames = yolov3Net.getLayerNames();

		// Get the names of the output layers in names
		names.resize(outLayers.size());
		for (size_t i = 0; i < outLayers.size(); ++i)
			names[i] = layersNames[outLayers[i] - 1];
	}
	return names;
}

void DetectorNet::postProcess() {
	vector<Rect> boxes;
	for (size_t i = 0; i < outs.size(); ++i)
	{
		// Scan through all the bounding boxes output from the network and keep only the
		// ones with high confidence scores. Assign the box's class label as the class
		// with the highest score for the box.
		float* data = (float*)outs[i].data;
		for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
		{
			Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
			Point classIdPoint;
			double confidence;
			// Get the value and location of the maximum score
			minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
			if (confidence > confThreshold)
			{
				int centerX = (int)(data[0] * curFrame.cols);
				int centerY = (int)(data[1] * curFrame.rows);
				int width = (int)(data[2] * curFrame.cols);
				int height = (int)(data[3] * curFrame.rows);
				int left = centerX - width / 2;
				int top = centerY - height / 2;
				classIds.push_back(classIdPoint.x);
				confidences.push_back((float)confidence);
				boxes.push_back(Rect(left, top, width, height));
			}
		}
	}

	// Draw the predicted bounding box
	void DetectorNet::drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame)
	{
		//Draw a rectangle displaying the bounding box
		rectangle(frame, Point(left, top), Point(right, bottom), Scalar(255, 178, 50), 3);

		//Get the label for the class name and its confidence
		string label = format("%.2f", conf);
		if (!classes.empty())
		{
			CV_Assert(classId < (int)classes.size());
			label = classes[classId] + ":" + label;
		}

		//Display the label at the top of the bounding box
		int baseLine;
		Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
		top = max(top, labelSize.height);
		rectangle(frame, Point(left, top - round(1.5*labelSize.height)), Point(left + round(1.5*labelSize.width), top + baseLine), Scalar(255, 255, 255), FILLED);
		putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 0), 1);
	}
}
