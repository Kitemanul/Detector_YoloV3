// This code is written at BigVision LLC. It is based on the OpenCV project.
//It is subject to the license terms in the LICENSE file found in this distribution and at http://opencv.org/license.html

// Usage example:  ./object_detection_yolo.out --video=run.mp4
//                 ./object_detection_yolo.out --image=bird.jpg
#include "pch.h"
#include "Detector.h"

//互斥锁
mutex Thread_mutex;
mutex Thread_mutex1;
//队列1
deque<FrameDO> Buffer;
deque<NetResultDO> Buffer1;
deque<string> ImageName;

//帮助
extern const char* keys =
"{help h         |      | Usage examples: \n\t\t./Detector.exe --image=dog.jpg \n\t\t./Detector.exe --video=run_sm.mp4}"
"{image i        |<none>| input image   }"
"{video v        |<none>| input video   }"
"{rtsp r         |<none>|  rtsp url  }"
;
int main(int argc, char** argv)
{	
	int Interval;
	CommandLineParser parser(argc, argv, keys);
	parser.about("Use this script to run object detection using YOLO3 in OpenCV.");
	if (parser.has("help")||parser.has("h")){
		parser.printMessage();
		return 0;
	}
	//Load Cfg
	CfgLoader *Configuration = CfgLoader::instance();
	Configuration->init("Configuration.cfg");
	Configuration->getCfgByName(Interval,"DInterval");

	// Open a video file or an image file or a camera stream.	
	VideoCapture cap=OpenInputFile(parser);
	
	Mat frame;
	// Create a window
	static const string kWinName = "Deep learning object detection in OpenCV";
	namedWindow(kWinName, WINDOW_NORMAL);

	//两次检测之间间隔帧数,默认每秒处理一帧
	int i = 0;
	int flag=0;
	int slot=0;

	//// 帧处理线程.
	thread FrameThread(ThreadProcessFrame);
	FrameThread.detach();
	//// 数据库操作线程.
	thread DBThread(ThreadDBOperate);
	DBThread.detach();

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
				FrameDO fdo(Frame, db_Operator::get_CurrentTime_s());
				Buffer.push_back(fdo);	
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
	cap.release();
	return 0;
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
			//FPS = cap.get(CV_CAP_PROP_FPS);
		
		}
		// Open the webcamstream
		else
		{
			cap.open(parser.get<String>("rtsp"));
			//FPS = cap.get(CV_CAP_PROP_FPS);
		}
	}
	catch (...) {
		cout << "Could not open the input image/video stream" << endl;
		return 0;
	}
	return cap;
}

//帧处理线程调用函数
void ThreadProcessFrame() {
	ProcessFrame threadProcessFrame;
	threadProcessFrame.ThreadProcessFrame();
}

//数据库操作线程调用函数
void ThreadDBOperate() {
	DBOperator threadDBOperator;
	threadDBOperator.threadInsertAlertInf();

}



