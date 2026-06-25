// Entry point for the YOLOv3 PPE safety compliance detector.
//
// Usage:
//   Detector --image=dog.jpg
//   Detector --video=run.mp4
//   Detector --rtsp=rtsp://user:pass@host:port/stream
//
// Based on the YOLOv3-in-OpenCV object detection sample by BigVision LLC,
// distributed under the OpenCV license (http://opencv.org/license.html).

#include <csignal>
#include <memory>
#include <string>

#include <opencv2/core/utility.hpp>

#include "detector/capture/FrameSource.h"
#include "detector/core/Config.h"
#include "detector/core/Log.h"
#include "detector/persistence/FileSink.h"
#include "detector/pipeline/Pipeline.h"

#if defined(DETECTOR_WITH_SQLSERVER) && defined(_WIN32)
#include "detector/persistence/SqlServerSink.h"
#endif

using namespace detector;

namespace {

const char* kKeys =
    "{help h        |       | print this message }"
    "{image i       |       | input image  }"
    "{video v       |       | input video  }"
    "{rtsp r        |       | rtsp url     }";

void handleSignal(int) {
    Pipeline::requestStop();
}

// Pick the persistence backend: SQL Server when it is compiled in, otherwise
// the portable file sink.
std::unique_ptr<ISink> makeSink(CfgLoader& cfg, const std::string& outputDir) {
#if defined(DETECTOR_WITH_SQLSERVER) && defined(_WIN32)
    std::string dataSource, dbName, dbUser, dbPassword;
    cfg.getCfgByName(dataSource, "DataSource");
    cfg.getCfgByName(dbName, "DB_Name");
    cfg.getCfgByName(dbUser, "DB_User");
    cfg.getCfgByName(dbPassword, "DB_Password");
    return std::make_unique<SqlServerSink>(dbName, dbUser, dbPassword, dataSource, outputDir);
#else
    (void)cfg;
    return std::make_unique<FileSink>(outputDir);
#endif
}

} // namespace

int main(int argc, char** argv) {
    cv::CommandLineParser parser(argc, argv, kKeys);
    parser.about("YOLOv3 PPE (helmet / reflective vest) safety compliance detector.");
    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    CfgLoader* cfg = CfgLoader::instance();
    if (cfg->init("Configuration.cfg") != 0) {
        LOG(Error) << "Could not open Configuration.cfg";
        return 1;
    }

    PipelineConfig pcfg;
    cfg->getCfgByName(pcfg.interval, "Interval");
    cfg->getCfgByName(pcfg.outputDir, "DetectedFrameDir");

    // Resolve the input source from the command line.
    SourceType type;
    std::string uri;
    if (parser.has("image")) {
        type = SourceType::Image;
        uri = parser.get<std::string>("image");
    } else if (parser.has("video")) {
        type = SourceType::Video;
        uri = parser.get<std::string>("video");
    } else if (parser.has("rtsp")) {
        type = SourceType::Rtsp;
        uri = parser.get<std::string>("rtsp");
    } else {
        LOG(Error) << "No input given. Use --image, --video or --rtsp (see --help).";
        return 1;
    }

    std::signal(SIGINT, handleSignal);

    try {
        auto source = std::make_unique<FrameSource>(type, uri);
        auto sink = makeSink(*cfg, pcfg.outputDir);
        Pipeline pipeline(std::move(source), std::move(sink), pcfg);
        pipeline.run();
    } catch (const std::exception& e) {
        LOG(Error) << e.what();
        return 1;
    }

    return 0;
}
