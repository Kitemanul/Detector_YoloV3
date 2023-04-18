#include "pch.h"
#include "DetectorNet.h"



void DetectorNet::loadConfig()
{	
	//Load Cfg
	cfgReader.init("Configuration.cfg");
	cfgReader.getCfgByName(confThreshold, "Threshold");
	// Load names of classes 00 01 10 11
	string classesFile = pro_dir + "voc.names";
	ifstream ifs(classesFile.c_str());
	string line;
	while (getline(ifs, line)) classes.push_back(line);

}

DetectorNet::DetectorNet()
{
	loadConfig();
	// Load the network
	yolov3Net = dnn::readNetFromDarknet(modelConfiguration, modelWeights);
	yolov3Net.setPreferableBackend(dnn::DNN_BACKEND_OPENCV);
	yolov3Net.setPreferableTarget(dnn::DNN_TARGET_CPU);
}

void DetectorNet::process(Mat frame)
{
	// Create a 4D blob from a frame.
	blobFromImage(frame, blob, 1 / 255.0, cvSize(inpWidth, inpHeight), Scalar(0, 0, 0), true, false);
	//Sets the input to the network
	yolov3Net.setInput(blob);
	// Runs the forward pass to get output of the output layers	
	yolov3Net.forward(outs, getOutputsNames(net));
}

void DetectorNet::postProcess()
{
}
