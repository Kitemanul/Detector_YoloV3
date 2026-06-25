#include "detector/pipeline/Pipeline.h"

#include <opencv2/highgui.hpp>

#include "detector/core/Log.h"
#include "detector/core/Time.h"
#include "detector/inference/DetectorNet.h"

namespace detector {

std::atomic<bool> Pipeline::stopRequested_{false};

Pipeline::Pipeline(std::unique_ptr<FrameSource> source, std::unique_ptr<ISink> sink, PipelineConfig cfg)
    : source_(std::move(source)),
      sink_(std::move(sink)),
      cfg_(std::move(cfg)),
      frameQ_(8),    // small: keep latency low for live streams
      resultQ_(16) {}

Pipeline::~Pipeline() {
    // Defensive: make sure the queues are closed and the workers are joined
    // even if run() exited abnormally.
    frameQ_.close();
    resultQ_.close();
    if (inferenceThread_.joinable()) inferenceThread_.join();
    if (persistenceThread_.joinable()) persistenceThread_.join();
}

void Pipeline::requestStop() {
    stopRequested_.store(true);
}

void Pipeline::run() {
    stopRequested_.store(false);
    inferenceThread_ = std::thread(&Pipeline::inferenceLoop, this);
    persistenceThread_ = std::thread(&Pipeline::persistenceLoop, this);

    const std::string winName = "Detector";
    if (cfg_.display) {
        cv::namedWindow(winName, cv::WINDOW_NORMAL);
    }

    cv::Mat frame;
    int frameCounter = 0;
    int nextSampleSlot = 0;
    int readFailures = 0;
    const bool live = source_->isLive();

    LOG(Info) << "Pipeline started (interval=" << cfg_.interval
              << ", live=" << (live ? "yes" : "no") << ")";

    while (!stopRequested_.load()) {
        if (!source_->read(frame)) {
            if (++readFailures >= 10) {
                break; // end of stream
            }
            continue;
        }
        readFailures = 0;

        // Sample the first frame and then one every `interval` frames.
        if (frameCounter == 0 || frameCounter == nextSampleSlot) {
            frameCounter = 0;
            nextSampleSlot = cfg_.interval;
            Frame sampled(frame.clone(), nowForFilename());
            // Live streams drop stale frames; files apply backpressure so no
            // frame is lost.
            if (live) {
                frameQ_.pushDropOldest(std::move(sampled));
            } else {
                frameQ_.push(std::move(sampled));
            }
        }

        if (cfg_.display) {
            cv::imshow(winName, frame);
            if (cv::waitKey(30) == 27) { // Esc
                break;
            }
        }
        frameCounter++;
    }

    // Graceful shutdown: stop capture -> drain inference -> drain persistence.
    frameQ_.close();
    if (inferenceThread_.joinable()) inferenceThread_.join();
    resultQ_.close();
    if (persistenceThread_.joinable()) persistenceThread_.join();

    if (cfg_.display) {
        cv::destroyWindow(winName);
    }
    source_->release();
    LOG(Info) << "Pipeline stopped";
}

void Pipeline::inferenceLoop() {
    try {
        // The network is constructed exactly once and reused for every frame.
        DetectorNet net;
        Frame frame;
        while (frameQ_.waitPop(frame)) {
            net.compute(frame.image);
            resultQ_.push(Detection(net.classIds(), net.confidences(), frame.image, frame.timestamp));
        }
    } catch (const std::exception& e) {
        LOG(Error) << "Inference stage failed: " << e.what();
        stopRequested_.store(true);
        frameQ_.close();
    }
    // Source drained (or aborted): tell the persistence stage we are done.
    resultQ_.close();
}

void Pipeline::persistenceLoop() {
    try {
        sink_->open();
    } catch (const std::exception& e) {
        LOG(Error) << "Sink open failed: " << e.what();
        stopRequested_.store(true);
        // Drain the queue so the inference stage is never blocked on a full
        // result queue while we are shutting down.
        Detection drop;
        while (resultQ_.waitPop(drop)) {}
        return;
    }
    Detection det;
    while (resultQ_.waitPop(det)) {
        for (size_t i = 0; i < det.classIds.size(); ++i) {
            int level = classifyAlarmLevel(det.classIds[i]);
            if (level == 0) {
                continue; // compliant
            }
            AlarmRecord alarm;
            alarm.frame = det.frame;
            alarm.timestamp = det.timestamp;
            alarm.level = level;
            alarm.confidence = det.confidences[i];
            alarm.imageName = det.timestamp + "Warning" + std::to_string(level) +
                              "_" + std::to_string(i) + ".jpg";
            sink_->record(alarm);
        }
    }
    sink_->close();
}

// Map a detected class id to an alarm level (0 = compliant, no record).
int Pipeline::classifyAlarmLevel(int classId) {
    switch (classId) {
        case 0: return 1; // most severe
        case 1: return 2;
        case 2: return 3;
        default: return 0;
    }
}

} // namespace detector
