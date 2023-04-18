#include "pch.h"
#include "DetectorNet.h"



DetectorNet::DetectorNet()
{	
	// Load names of classes
	string classesFile = pro_dir + "voc.names";
	ifstream ifs(classesFile.c_str());
	string line;
	while (getline(ifs, line)) classes.push_back(line);

	// Give the configuration and weight files for the model
	String modelConfiguration = pro_dir + "yolov3-voc.cfg";
	String modelWeights = pro_dir + "yolov3-voc_6000.weights";

	// Load the network
	yolov3Net = readNetFromDarknet(modelConfiguration, modelWeights);
	yolov3Net.setPreferableBackend(DNN_BACKEND_OPENCV);
	yolov3Net.setPreferableTarget(DNN_TARGET_CPU);
}
