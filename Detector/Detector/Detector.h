#pragma once
#include "pch.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <deque>
#include <mutex>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include "Configuration.h"
#include "DBOperator.h"
#include "ProcessFrame.h"
#include "FrameDO.h"
#include "NetResultDO.h"

using namespace cv;
using namespace dnn;
using namespace std;

// Mutexes guarding the inter-thread queues.
extern mutex Thread_mutex;
extern mutex Thread_mutex1;

// Capture -> inference queue.
extern deque<FrameDO> Buffer;

// Inference -> database queue.
extern deque<NetResultDO> Buffer1;

// Open the input source (image / video / RTSP) described on the command line.
VideoCapture OpenInputFile(CommandLineParser parser);

// Thread entry point: consumes frames and runs YOLOv3 inference.
void ThreadProcessFrame();

// Thread entry point: consumes inference results and writes them to the database.
void ThreadDBOperate();
