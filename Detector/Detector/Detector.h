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


//������
extern mutex Thread_mutex;
extern mutex Thread_mutex1;
//ץ֡-���� ���� 
extern deque<FrameDO> Buffer;

//����-���ݿ� ����  
extern deque<NetResultDO> Buffer1;

// Load InputFile
VideoCapture OpenInputFile(CommandLineParser parser);

////�̵߳��ú��� ������Ƶ֡
void ThreadProcessFrame();
////�̵߳��ú��� �������ݿ�
void ThreadDBOperate();