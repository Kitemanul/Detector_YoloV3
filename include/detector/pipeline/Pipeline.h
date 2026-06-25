#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "detector/capture/FrameSource.h"
#include "detector/core/ThreadSafeQueue.h"
#include "detector/inference/Detection.h"
#include "detector/inference/Frame.h"
#include "detector/persistence/ISink.h"

namespace detector {

struct PipelineConfig {
    int interval = 24;       // frames between two sampled frames
    std::string outputDir;   // directory for violation snapshots (trailing sep)
    bool display = false;    // show the live window (requires a GUI)
};

// Orchestrates the three-stage capture -> inference -> persistence pipeline.
//
// Capture runs on the calling thread (run()); inference and persistence each
// run on their own worker thread. The stages communicate through bounded,
// condition-variable-backed queues, and end-of-stream is propagated by closing
// each queue in turn, giving a clean, deterministic shutdown with all threads
// joined. A SIGINT handler can request an early, graceful stop.
class Pipeline {
public:
    Pipeline(std::unique_ptr<FrameSource> source, std::unique_ptr<ISink> sink, PipelineConfig cfg);
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    // Drive the pipeline to completion (returns when the source is exhausted or
    // a stop was requested and all stages have drained and joined).
    void run();

    // Request a cooperative stop (safe to call from a signal handler).
    static void requestStop();

private:
    void inferenceLoop();
    void persistenceLoop();
    static int classifyAlarmLevel(int classId);

    std::unique_ptr<FrameSource> source_;
    std::unique_ptr<ISink> sink_;
    PipelineConfig cfg_;

    ThreadSafeQueue<Frame> frameQ_;
    ThreadSafeQueue<Detection> resultQ_;

    std::thread inferenceThread_;
    std::thread persistenceThread_;

    static std::atomic<bool> stopRequested_;
};

} // namespace detector
