// Detector.cpp
//
// Application entry point for the YOLOv3 PPE (helmet / reflective vest) safety
// compliance detector.
//
// The program ingests an image, a video file, or an RTSP stream, samples one
// frame at a fixed interval, and pushes those frames through a three-stage
// pipeline implemented as a producer/consumer pipeline:
//
//   main thread            -> captures frames and enqueues them            (Buffer)
//   ThreadProcessFrame     -> runs YOLOv3 inference on each sampled frame   (Buffer -> Buffer1)
//   ThreadDBOperate        -> persists violations to SQL Server + on disk   (Buffer1)
//
// Usage:
//   ./Detector.exe --image=dog.jpg
//   ./Detector.exe --video=run.mp4
//   ./Detector.exe --rtsp=rtsp://user:pass@host:port/stream
//
// Based on the YOLOv3-in-OpenCV object detection sample by BigVision LLC,
// distributed under the OpenCV license (http://opencv.org/license.html).

#include "pch.h"
#include "Detector.h"

// Mutexes guarding the two inter-thread queues.
mutex Thread_mutex;   // protects Buffer  (capture -> inference)
mutex Thread_mutex1;  // protects Buffer1 (inference -> database)

// Inter-thread queues.
deque<FrameDO> Buffer;        // sampled frames awaiting inference
deque<NetResultDO> Buffer1;   // inference results awaiting persistence
deque<string> ImageName;

// Command line specification.
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
	if (parser.has("help") || parser.has("h")) {
		parser.printMessage();
		return 0;
	}

	// Load the runtime configuration (singleton).
	CfgLoader *Configuration = CfgLoader::instance();
	Configuration->init("Configuration.cfg");
	Configuration->getCfgByName(Interval, "DInterval");

	// Open the requested input: image, video file, or camera/RTSP stream.
	VideoCapture cap = OpenInputFile(parser);

	Mat frame;

	// Create a display window.
	static const string kWinName = "Deep learning object detection in OpenCV";
	namedWindow(kWinName, WINDOW_NORMAL);

	// Frame sampling state. By default one frame per second is forwarded.
	int frameCounter = 0;     // frames seen since the last sampled frame
	int readFailures = 0;     // consecutive failed reads (stream end detection)
	int nextSampleSlot = 0;   // frame index at which to sample the next frame

	// Start the inference worker thread.
	thread FrameThread(ThreadProcessFrame);
	FrameThread.detach();
	// Start the database worker thread.
	thread DBThread(ThreadDBOperate);
	DBThread.detach();

	for (;;)
	{
		if (!cap.read(frame))
		{
			// Tolerate a few transient read failures before treating the
			// stream as finished.
			if (++readFailures >= 10) {
				break;
			}
			continue;
		}

		// Sample the very first frame and then one frame every `Interval`
		// frames; hand the sampled frame off to the inference queue.
		if (frameCounter == 0 || frameCounter == nextSampleSlot)
		{
			Mat sampledFrame = frame.clone();
			frameCounter = 0;
			{
				lock_guard<mutex> lock(Thread_mutex);
				nextSampleSlot = Interval;
				Buffer.push_back(FrameDO(sampledFrame, db_Operator::get_CurrentTime_s()));
			}
		}

		imshow(kWinName, frame);
		waitKey(30);
		frameCounter++;
	}

	// Tear down.
	destroyWindow(kWinName);
	cap.release();
	return 0;
}

// Open the input source described by the command line arguments.
VideoCapture OpenInputFile(CommandLineParser parser)
{
	VideoCapture cap;
	string str;
	try {
		if (parser.has("image"))
		{
			// Open an image file.
			str = parser.get<String>("image");
			ifstream ifile(str);
			if (!ifile) throw("error");
			cap.open(str);
		}
		else if (parser.has("video"))
		{
			// Open a video file.
			str = parser.get<String>("video");
			ifstream ifile(str);
			if (!ifile) throw("error");
			cap.open(str);
		}
		else
		{
			// Open an RTSP / webcam stream.
			cap.open(parser.get<String>("rtsp"));
		}
	}
	catch (...) {
		cout << "Could not open the input image/video stream" << endl;
		return cap;
	}
	return cap;
}

// Entry point for the inference worker thread.
void ThreadProcessFrame() {
	ProcessFrame threadProcessFrame;
	threadProcessFrame.ThreadProcessFrame();
}

// Entry point for the database worker thread.
void ThreadDBOperate() {
	DBOperator threadDBOperator;
	threadDBOperator.threadInsertAlertInf();
}
