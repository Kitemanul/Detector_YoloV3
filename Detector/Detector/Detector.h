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
extern deque<string> Imagename;

//Load Configuration
CfgLoader *Configuration = CfgLoader::instance();



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