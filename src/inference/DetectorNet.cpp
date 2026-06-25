#include "detector/inference/DetectorNet.h"

#include <algorithm>
#include <fstream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>

#include "detector/core/Config.h"
#include "detector/core/Log.h"

namespace detector {

using namespace cv;
using namespace cv::dnn;

DetectorNet::DetectorNet() {
    loadConfig();
    // Load the network once.
    net_ = readNetFromDarknet(modelConfiguration_, modelWeights_);
    net_.setPreferableBackend(DNN_BACKEND_OPENCV);
    net_.setPreferableTarget(DNN_TARGET_CPU);
    LOG(Info) << "YOLOv3 network loaded from " << modelWeights_;
}

void DetectorNet::loadConfig() {
    CfgLoader* cfg = CfgLoader::instance();
    cfg->getCfgByName(confThreshold_, "Threshold");
    cfg->getCfgByName(modelDir_, "NNCfg_Dir");

    const std::string classesFile = modelDir_ + "voc.names";
    modelConfiguration_ = modelDir_ + "yolov3-voc.cfg";
    modelWeights_ = modelDir_ + "yolov3-voc_9000.weights";

    std::ifstream ifs(classesFile.c_str());
    std::string line;
    while (getline(ifs, line)) {
        classes_.push_back(line);
    }
}

void DetectorNet::compute(Mat& frame) {
    // Reset per-frame state (the instance is reused across frames).
    classIds_.clear();
    confidences_.clear();
    outs_.clear();

    curFrame_ = frame;
    blobFromImage(frame, blob_, 1 / 255.0, Size(inpWidth_, inpHeight_), Scalar(0, 0, 0), true, false);
    net_.setInput(blob_);
    net_.forward(outs_, outputNames());

    postProcess();
}

std::vector<String> DetectorNet::outputNames() {
    static std::vector<String> names;
    if (names.empty()) {
        std::vector<int> outLayers = net_.getUnconnectedOutLayers();
        std::vector<String> layersNames = net_.getLayerNames();
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i) {
            names[i] = layersNames[outLayers[i] - 1];
        }
    }
    return names;
}

void DetectorNet::postProcess() {
    std::vector<Rect> boxes;
    for (size_t i = 0; i < outs_.size(); ++i) {
        // Keep only the boxes with a high enough confidence; the class is the
        // one with the highest score for the box.
        float* data = (float*)outs_[i].data;
        for (int j = 0; j < outs_[i].rows; ++j, data += outs_[i].cols) {
            Mat scores = outs_[i].row(j).colRange(5, outs_[i].cols);
            Point classIdPoint;
            double confidence;
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > confThreshold_) {
                int centerX = (int)(data[0] * curFrame_.cols);
                int centerY = (int)(data[1] * curFrame_.rows);
                int width = (int)(data[2] * curFrame_.cols);
                int height = (int)(data[3] * curFrame_.rows);
                int left = centerX - width / 2;
                int top = centerY - height / 2;
                classIds_.push_back(classIdPoint.x);
                confidences_.push_back((float)confidence);
                boxes.push_back(Rect(left, top, width, height));
            }
        }
    }

    // Non-maximum suppression to drop redundant overlapping boxes.
    std::vector<int> indices;
    NMSBoxes(boxes, confidences_, confThreshold_, nmsThreshold_, indices);

    // Rebuild the result vectors so that they line up with the surviving boxes.
    std::vector<int> keptIds;
    std::vector<float> keptConf;
    keptIds.reserve(indices.size());
    keptConf.reserve(indices.size());
    for (size_t i = 0; i < indices.size(); ++i) {
        int idx = indices[i];
        Rect box = boxes[idx];
        drawPred(classIds_[idx], confidences_[idx], box.x, box.y,
                 box.x + box.width, box.y + box.height, curFrame_);
        keptIds.push_back(classIds_[idx]);
        keptConf.push_back(confidences_[idx]);
    }
    classIds_ = std::move(keptIds);
    confidences_ = std::move(keptConf);
}

void DetectorNet::drawPred(int classId, float conf, int left, int top, int right, int bottom, Mat& frame) {
    rectangle(frame, Point(left, top), Point(right, bottom), Scalar(255, 178, 50), 3);

    std::string label = format("%.2f", conf);
    if (!classes_.empty()) {
        CV_Assert(classId < (int)classes_.size());
        label = classes_[classId] + ":" + label;
    }

    int baseLine;
    Size labelSize = getTextSize(label, FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
    top = std::max(top, labelSize.height);
    rectangle(frame, Point(left, top - cvRound(1.5 * labelSize.height)),
              Point(left + cvRound(1.5 * labelSize.width), top + baseLine),
              Scalar(255, 255, 255), FILLED);
    putText(frame, label, Point(left, top), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 0), 1);
}

} // namespace detector
