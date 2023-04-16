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
//������
std::mutex Thread_mutex;


vector<string> classes;
deque<Mat> Buffer;
deque<string> Imagename;


//�����ļ� 
//voc.names
//yolov3-voc.cfg
//yolov3-voc_11400.weights
//�����ļ�Ŀ¼��Ĭ����Ŀ��Ŀ¼
string pro_dir = "";

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