
#include "ProcessFrame.h"
#include "pch.h"

void ProcessFrame::postprocess(Mat& frame, const vector<Mat>& outs, vector<int>& classIds, vector<float>& confidences){
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
void ProcessFrame::drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame)
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
vector<String> ProcessFrame::getOutputsNames(const Net& net)
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
void ProcessFrame::ThreadProcessFrame()
{

	Configuration->getCfgByName(pro_dir, "NNCfg_Dir");  //get神经网络配置文件目录
	//新建数据库操作对象
	dbo = new db_Operator("sql server", DB_Name, DB_User, DB_Password, DataSource);
	Net net = LoadNetCfg();
	// Put efficiency information. The function getPerfProfile returns the overall time for inference(t) and the timings for each of the layers(in layersTimes)

	Mat frame;
	Mat Frame;

	string ImageName;
	int flag = 0;
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
			frame = NULL;
			flag++;
		}
		Thread_mutex.unlock();
		if (!frame.empty())
		{

			Process(frame, net, ImageName);

		}

		if (flag == 10)
		{
			break;
		}

	}
}

void ProcessFrame::Process(Mat frame, Net net, string ImageName)
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
int ProcessFrame::ProcessClass(vector<int>& classIds, int classid)
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




