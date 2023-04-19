#include "pch.h"
#include "FrameDO.h"

FrameDO::FrameDO(Mat frame, string ts)
{
	this->frame = frame;
	this->timestamp = ts;
}

Mat FrameDO::getFrame()
{
	return frame;
}

void FrameDO::setFrame(Mat & frame)
{
	this->frame = frame;
}

string FrameDO::getTimeStamp()
{
	return timestamp;
}

void FrameDO::setTimeStamp(string ts)
{
	this->timestamp = ts;
}
