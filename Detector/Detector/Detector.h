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


//ª•≥‚À¯
extern std::mutex Thread_mutex;
//∂”¡–1
extern deque<Mat> Buffer;
extern deque<string> Imagename;

// Load InputFile
VideoCapture OpenInputFile(CommandLineParser parser);