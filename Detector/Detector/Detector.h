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

// Initialize the parameters
float confThreshold; // Confidence threshold
float nmsThreshold = 0.4;  // Non-maximum suppression threshold
int inpWidth = 416;  // Width of network's input image
int inpHeight = 416; // Height of network's input image
//互斥锁
std::mutex Thread_mutex;


vector<string> classes;
deque<Mat> Buffer;
deque<string> Imagename;


//配置文件 
//voc.names
//yolov3-voc.cfg
//yolov3-voc_11400.weights
//配置文件目录，默认项目根目录
string pro_dir = "";

//违规截图存储路径，默认项目根目录
string DirOfDetectedFrame = "";

//视频流帧率
float FPS;
//两次检测之间间隔时间
float Interval;
//成功检测到目标后，下一次检测间隔时间
float DInterval;
float slot;

//数据库配置
string DB_Name;
string DB_User;
string DB_Password;
string DataSource;
db_Operator *dbo;