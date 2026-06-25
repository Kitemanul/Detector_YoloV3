#pragma once
#include "pch.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <deque>
#include <mutex>

#include "Configuration.h"
#include "db_Operator.h"
#include "ProcessFrame.h"
#include "Detector.h"
#include "DetectorNet.h"

// Inference worker. Pulls sampled frames off the capture queue, runs YOLOv3
// inference on them, and pushes the results onto the database queue.
class ProcessFrame {

public:
	// Output directory for violation snapshots.
	string dirofdetectedframe;
	// Source frame rate.
	float fps;
	// Frame interval between two consecutive detections.
	float Interval;
	// Frame interval applied after a target was successfully detected.
	float DInterval;

	CfgLoader * cfgReader;

	ProcessFrame();

	void ThreadProcessFrame();

	void Process(Mat &frame, string ImageName);
};
