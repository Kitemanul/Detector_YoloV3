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
#include "db_Operator.h"
#include "ProcessFrame.h"

using namespace cv;
using namespace dnn;
using namespace std;


//互斥锁
extern std::mutex Thread_mutex;
//队列1
extern deque<Mat> Buffer;
extern deque<string> ImageName;

// Load InputFile
VideoCapture OpenInputFile(CommandLineParser parser);

////线程调用函数 处理视频帧
void ThreadProcessFrame();