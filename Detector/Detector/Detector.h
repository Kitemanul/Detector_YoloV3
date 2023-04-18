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


//������
extern std::mutex Thread_mutex;
//����1
extern deque<Mat> Buffer;
extern deque<string> Imagename;

//Load Configuration
CfgLoader *Configuration = CfgLoader::instance();



//Υ���ͼ�洢·����Ĭ����Ŀ��Ŀ¼
string DirOfDetectedFrame = "";

//��Ƶ��֡��
float FPS;
//���μ��֮����ʱ��
float Interval;
//�ɹ���⵽Ŀ�����һ�μ����ʱ��
float DInterval;
float slot;

//���ݿ�����
string DB_Name;
string DB_User;
string DB_Password;
string DataSource;
db_Operator *dbo;