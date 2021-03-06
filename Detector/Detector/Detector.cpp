// This code is written at BigVision LLC. It is based on the OpenCV project.
//It is subject to the license terms in the LICENSE file found in this distribution and at http://opencv.org/license.html

// Usage example:  ./object_detection_yolo.out --video=run.mp4
//                 ./object_detection_yolo.out --image=bird.jpg

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

const char* keys =
"{help h         |      | Usage examples: \n\t\t./Detector.exe --image=dog.jpg \n\t\t./Detector.exe --video=run_sm.mp4}"
"{image i        |<none>| input image   }"
"{video v        |<none>| input video   }"
"{rtsp r         |<none>|  rtsp url  }"
;
using namespace cv;
using namespace dnn;
using namespace std;

//Load Configuration
CfgLoader *Configuration = CfgLoader::instance();

// Initialize the parameters
float confThreshold; // Confidence threshold
float nmsThreshold = 0.4;  // Non-maximum suppression threshold
int inpWidth = 416;  // Width of network's input image
int inpHeight = 416; // Height of network's input image
//互斥锁
std::mutex Thread_mutex;
std::mutex _mutex;

vector<string> classes;
deque<Mat> Buffer;
deque<string> Imagename;

//Load net Configuration
Net LoadNetCfg();

//Open image/video/cam 
VideoCapture OpenInputFile(CommandLineParser parser);

// Remove the bounding boxes with low confidence using non-maxima suppression
void postprocess(Mat& frame, const vector<Mat>& out, vector<int>& classIds,vector<float>& confidences);

// Draw the predicted bounding box
void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame);

// Get the names of the output layers
vector<String> getOutputsNames(const Net& net);

//Process frame
void ThreadProcessFrame();
void ProcessFrame(Mat frame, Net net,string ImageName);

//Peocess Class 
//如果检测到违规的目标，截图
int ProcessClass(vector<int>& classIds, int classid);

//配置文件 
//voc.names
//yolov3-voc.cfg
//yolov3-voc_11400.weights
//配置文件目录，默认项目根目录
string pro_dir = ""; 

//违规截图存储路径，默认项目根目录
string DirOfDetectedFrame = "";

//视频流帧率
float FPS ;
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

int main(int argc, char** argv)
{
	CommandLineParser parser(argc, argv, keys);
	parser.about("Use this script to run object detection using YOLO3 in OpenCV.");
	if (parser.has("help")||parser.has("h"))
	{
		parser.printMessage();
		return 0;
	}

	//Load Cfg
	Configuration->init("Configuration.cfg");
	Configuration->getCfgByName(DirOfDetectedFrame, "DetectedFrameDir");//get违规图片保存目录
	Configuration->getCfgByName(Interval, "Interval");  
	Configuration->getCfgByName(DInterval, "DInterval");
	//load database cfg
	Configuration->getCfgByName(DataSource, "DataSource");
	Configuration->getCfgByName(DB_Name, "DB_Name");
	Configuration->getCfgByName(DB_User, "DB_User");
	Configuration->getCfgByName(DB_Password, "DB_Password");
	Configuration->getCfgByName(confThreshold, "Threshold");

	// Open a video file or an image file or a camera stream.	
	VideoCapture cap=OpenInputFile(parser);
	
	Mat frame;
	// Create a window
	static const string kWinName = "Deep learning object detection in OpenCV";
	namedWindow(kWinName, WINDOW_NORMAL);

	//两次检测之间间隔帧数,默认每秒处理一帧
	int i = 0;
	int flag=0;
	// Process frames.
	thread Thread(ThreadProcessFrame);
	Thread.detach();

	for(;;)
	{    
		
		if (cap.read(frame))
		{
			if (i == slot || i == 0)
			{   
				Mat Frame=frame.clone();
				i = 0;
				Thread_mutex.lock();
				slot = Interval;
				Buffer.push_back(Frame);
				Imagename.push_back(db_Operator::get_CurrentTime_s());
				Thread_mutex.unlock();
				
			};											
			imshow(kWinName, frame);
			waitKey(30);
			i++;
		}
		else if (!cap.read(frame))
		{
			flag ++;
		}
		if (flag == 10)
		{
			break;
		}
	}
	//等待所有线程结束
	destroyWindow(kWinName);
	//Thread.join();
	cap.release();
	return 0;
}

//Load Net Configurantion

Net LoadNetCfg()
{   
	// Load names of classes
	string classesFile = pro_dir + "voc.names";
	ifstream ifs(classesFile.c_str());
	string line;
	while (getline(ifs, line)) classes.push_back(line);

	// Give the configuration and weight files for the model
	String modelConfiguration = pro_dir + "yolov3-voc.cfg";
	String modelWeights = pro_dir + "yolov3-voc_6000.weights";

	// Load the network
	Net net = readNetFromDarknet(modelConfiguration, modelWeights);
	net.setPreferableBackend(DNN_BACKEND_OPENCV);
	net.setPreferableTarget(DNN_TARGET_CPU);

	return net;

}


// Load InputFile
VideoCapture OpenInputFile(CommandLineParser parser)
{   
	VideoCapture cap;
	string str;
	try {

		
		if (parser.has("image"))
		{
			// Open the image file
			str = parser.get<String>("image");
			ifstream ifile(str);
			if (!ifile) throw("error");
			cap.open(str);
			
		}
		else if (parser.has("video"))
		{
			// Open the video file
			str = parser.get<String>("video");
			ifstream ifile(str);
			if (!ifile) throw("error");
			cap.open(str);
			FPS = cap.get(CV_CAP_PROP_FPS);
		
		}
		// Open the webcamstream
		else
		{
			cap.open(parser.get<String>("rtsp"));
			FPS = cap.get(CV_CAP_PROP_FPS);
		}

	}
	catch (...) {
		cout << "Could not open the input image/video stream" << endl;
		return 0;
	}
	return cap;
}



// Remove the bounding boxes with low confidence using non-maxima suppression
void postprocess(Mat& frame, const vector<Mat>& outs, vector<int>& classIds, vector<float>& confidences)
{

	vector<Rect> boxes;
	for (size_t i = 0; i < outs.size(); ++i)
	{
		// Scan through all the bounding boxes output from the network and keep only the
		// ones with high confidence scores. Assign the box's class label as the class
		// with the highest score for the box.
		float* data = (float*)outs[i].data;
		for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
		{
			Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
			Point classIdPoint;
			double confidence;
			// Get the value and location of the maximum score
			minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
			if (confidence > confThreshold)
			{
				int centerX = (int)(data[0] * frame.cols);
				int centerY = (int)(data[1] * frame.rows);
				int width = (int)(data[2] * frame.cols);
				int height = (int)(data[3] * frame.rows);
				int left = centerX - width / 2;
				int top = centerY - height / 2;
				classIds.push_back(classIdPoint.x);
				confidences.push_back((float)confidence);
				boxes.push_back(Rect(left, top, width, height));
			}
		}
	}

	// Perform non maximum suppression to eliminate redundant overlapping boxes with
	// lower confidences
	vector<int> indices;
	NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
	for (size_t i = 0; i < indices.size(); ++i)
	{
		int idx = indices[i];
		Rect box = boxes[idx];
		drawPred(classIds[idx], confidences[idx], box.x, box.y,
			box.x + box.width, box.y + box.height, frame);
	}
}

// Draw the predicted bounding box
void drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame)
{
	//Draw a rectangle displaying the bounding box
	rectangle(frame, Point(left, top), Point(right, bottom), Scalar(255, 178, 50), 3);

	//Get the label for the class name and its confidence
	string label = format("%.2f", conf);
	if (!classes.empty())
	{
		CV_Assert(classId < (int)classes.size());
		label = classes[classId] + ":" + label;
	}

	//Display the label at the top of the bounding box
	int baseLine;
	Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
	top = max(top, labelSize.height);
	rectangle(frame, Point(left, top - round(1.5*labelSize.height)), Point(left + round(1.5*labelSize.width), top + baseLine), Scalar(255, 255, 255), FILLED);
	putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 0), 1);
}

// Get the names of the output layers
vector<String> getOutputsNames(const Net& net)
{
	static vector<String> names;
	if (names.empty())
	{
		//Get the indices of the output layers, i.e. the layers with unconnected outputs
		vector<int> outLayers = net.getUnconnectedOutLayers();

		//get the names of all the layers in the network
		vector<String> layersNames = net.getLayerNames();

		// Get the names of the output layers in names
		names.resize(outLayers.size());
		for (size_t i = 0; i < outLayers.size(); ++i)
			names[i] = layersNames[outLayers[i] - 1];
	}
	return names;
}

//帧处理线程
void ThreadProcessFrame()
{   
	
	Configuration->getCfgByName(pro_dir, "NNCfg_Dir");  //get神经网络配置文件目录
	//新建数据库操作对象
	dbo = new db_Operator("sql server", DB_Name, DB_User, DB_Password, DataSource);
	Net net = LoadNetCfg();
	// Put efficiency information. The function getPerfProfile returns the overall time for inference(t) and the timings for each of the layers(in layersTimes)
	
	Mat frame;
	Mat Frame;

	string ImageName ;
	int flag=0;
	while (1)
	{
		Thread_mutex.lock();
		if (!Buffer.empty())
		{
		   flag = 0;
           frame = Buffer.front();
		   ImageName = Imagename.front();
		   Buffer.pop_front();
		   Imagename.pop_front();
		}	
		else
		{
			frame =NULL;
			flag++;
		}
		Thread_mutex.unlock();
		if (!frame.empty())
		{   
			
			ProcessFrame(frame, net, ImageName);	
		
		}
		
		if (flag == 10)
		{
			break;
		}
		
	}
}

void ProcessFrame(Mat frame, Net net,string ImageName)
{   
	vector<Mat> outs;
	Mat blob;
	Mat Frame;
	//检测到的标签及置信度
	vector<int> classIds;
	vector<float> confidences;
	vector<double> layersTimes;
	string label;
	double freq = getTickFrequency() / 1000;;
	double t;

	// Create a 4D blob from a frame.
	blobFromImage(frame, blob, 1 / 255.0, cvSize(inpWidth, inpHeight), Scalar(0, 0, 0), true, false);
	//Sets the input to the network
	net.setInput(blob);
	// Runs the forward pass to get output of the output layers	
	net.forward(outs, getOutputsNames(net));
	// Remove the bounding boxes with low confidence	   
	postprocess(frame, outs, classIds, confidences);
	t = net.getPerfProfile(layersTimes) / freq;
	label = format("Inference time for a frame : %.2f ms", t);
	putText(frame, label, Point(0, 15), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 0, 255));
	Frame = frame.clone();
	//process class and confidence
	for (int i = 0; i < classIds.size(); i++)
	{
		switch (ProcessClass(classIds, i))
		{
		case 1:
			imwrite(DirOfDetectedFrame + ImageName + "Warning1" + "_" + to_string(i) + ".jpg", Frame);
			dbo->db_InsertRecord(confidences[i], 1, DirOfDetectedFrame, ImageName + "Warning1" + "_" + to_string(i) + ".jpg");
			slot = DInterval;
			cout << "Save Detected Frame!" << endl;
			break;
		case 2:
			imwrite(DirOfDetectedFrame + ImageName + "Warning2" + "_" + to_string(i) + ".jpg", Frame);
			dbo->db_InsertRecord(confidences[i], 2, DirOfDetectedFrame, ImageName + "Warning2" + "_" + to_string(i) + ".jpg");
			slot = DInterval;
			cout << "Save Detected Frame!" << endl;
			break;
		case 3:
			imwrite(DirOfDetectedFrame + ImageName + "Warning3" + "_" + to_string(i) + ".jpg", Frame);
			dbo->db_InsertRecord(confidences[i], 3, DirOfDetectedFrame, ImageName + "Warning3" + "_" + to_string(i) + ".jpg");
			slot = DInterval;
			cout << "Save Detected Frame!" << endl;
			break;
		case 0:break;
		}
	}
}

//如果检测到违规的目标，返回true
int ProcessClass(vector<int>& classIds,int classid)
{   
	   
	switch (classIds[classid])
	{
		  case 0:return 1;  //一级警告
			     
		  case 1:return 2;  //二级警告
			    
		  case 2:return 3;  //三级警告
			   
		  default:return 0; //正常			     
	}	
	
	return false;
}

