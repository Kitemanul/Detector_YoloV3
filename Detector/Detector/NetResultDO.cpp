#include "pch.h"
#include "NetResultDO.h"

NetResultDO::NetResultDO(vector<int> classIds, vector<float> confidences, vector<double> layersTimes,Mat frame,string ts)
{
	this->classIds = classIds;
	this->confidences = confidences;
	this->layersTimes = layersTimes;
	this->timestamp = ts;
	this->frame = frame;
}


vector<int> NetResultDO::getClssIds()
{
	return classIds;
}

vector<float> NetResultDO::getConfidences()
{
	return confidences;
}

vector<double> NetResultDO::getLayersTimes()
{
	return layersTimes;
}

Mat NetResultDO::getFrame()
{
	return frame;
}

string NetResultDO::getTs()
{
	return timestamp;
}

void NetResultDO::setClassIds(vector<int> classIds)
{
	classIds = classIds;
}

void NetResultDO::setConfidences(vector<float> confidences)
{
	confidences = confidences;
}

void NetResultDO::setLayersTimes(vector<double> layersTimes)
{
	layersTimes = layersTimes;
}
