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


//互斥锁
extern mutex Thread_mutex;
extern mutex Thread_mutex1;
//抓帧-处理 队列 
extern deque<FrameDO> Buffer;

//处理-数据库 队列  
extern deque<NetResultDO> Buffer1;

// Load InputFile
VideoCapture OpenInputFile(CommandLineParser parser);

////线程调用函数 处理视频帧
void ThreadProcessFrame();
////线程调用函数 操作数据库
void ThreadDBOperate();