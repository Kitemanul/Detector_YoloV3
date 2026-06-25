# PPE Safety Compliance Detector (YOLOv3 + C++/OpenCV)

A real-time **personal protective equipment (PPE) compliance detector** for
industrial/construction safety monitoring. It analyses an image, a video file,
or a live **RTSP** camera stream, detects whether each person is wearing a
**safety helmet** and a **reflective vest**, and automatically captures and
records every violation.

The application is written in modern C++ on top of the **OpenCV DNN** module
(YOLOv3 / Darknet) and is built around a multithreaded **producer/consumer
pipeline** so that real-time capture is never blocked by inference or by
database I/O.

<p>
  <img alt="language" src="https://img.shields.io/badge/language-C%2B%2B-00599C">
  <img alt="opencv" src="https://img.shields.io/badge/OpenCV-3.4.3-5C3EE8">
  <img alt="model" src="https://img.shields.io/badge/model-YOLOv3-00FFFF">
  <img alt="platform" src="https://img.shields.io/badge/platform-Windows-0078D6">
  <img alt="license" src="https://img.shields.io/badge/license-MIT-green">
</p>

---

## Highlights

- **Real-time inference** on image / video / RTSP inputs, with configurable
  frame sampling (≈1 frame per second by default).
- **Three-stage concurrent pipeline** (capture → inference → persistence) using
  `std::thread`, mutex-guarded queues and RAII locking — a slow database write
  cannot stall the camera stream.
- **YOLOv3 object detection** via OpenCV's DNN backend, including confidence
  filtering and non-maximum suppression.
- **Tiered alarms**: each detection is mapped to a severity level; violations
  are annotated, snapshotted to disk, and persisted to **SQL Server**.
- **External configuration** via a simple `.cfg` file — thresholds, paths,
  sampling intervals and database credentials require no recompilation.

## Architecture

```
 input (image/video/RTSP)
        │
        ▼
 ┌─────────────┐  FrameDO   ┌──────────────────┐  NetResultDO  ┌────────────────┐
 │  capture    │ ─────────► │  YOLOv3 inference │ ────────────► │  persistence   │
 │ (main loop) │   Buffer   │  (ProcessFrame)   │    Buffer1    │  (DBOperator)  │
 └─────────────┘            └──────────────────┘               └────────────────┘
                                                                  │        │
                                                            snapshot.jpg   SQL Server
```

See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for the full design, component
breakdown, and the class → alarm-level mapping.

## Tech stack

| Area | Technology |
| --- | --- |
| Language | C++ (VS2017 toolset `v141`, Unicode) |
| Computer vision | OpenCV 3.4.3 (DNN, imgproc, highgui) |
| Model | YOLOv3 (Darknet `.cfg` + `.weights`) |
| Concurrency | `std::thread`, `std::mutex`, `std::deque` |
| Storage | SQL Server 2017 via ADO (`msado15`) |
| Build | Visual Studio solution (`Detector.sln`) |

## Project structure

```
Detector_YoloV3/
├── Detector/Detector/
│   ├── Detector.{h,cpp}        # entry point + capture loop + thread bootstrap
│   ├── ProcessFrame.{h,cpp}    # inference worker thread
│   ├── DetectorNet.{h,cpp}     # OpenCV DNN / YOLOv3 wrapper
│   ├── DBOperator.{h,cpp}      # persistence worker thread
│   ├── db_Operator.{h,cpp}     # low-level SQL Server (ADO) access
│   ├── Configuration.{h,cpp}   # .cfg reader (singleton)
│   ├── FrameDO.{h,cpp}         # capture→inference queue item
│   ├── NetResultDO.{h,cpp}     # inference→persistence queue item
│   ├── Configuration.cfg       # runtime configuration
│   └── Detector.sln            # Visual Studio solution
├── docs/ARCHITECTURE.md
├── CHANGELOG.md
└── LICENSE
```

## Prerequisites

- **Windows** with **Visual Studio 2017** (or newer with the `v141` toolset).
- **OpenCV 3.4.3** (the project links `opencv_world343.lib` / `opencv_world343d.lib`).
- **SQL Server 2017** (and `msado15.dll`, normally at
  `C:\Program Files\Common Files\System\ADO\`) — required only for the database
  persistence feature.
- YOLOv3 model files (not included in the repository):
  - `voc.names` — class labels
  - `yolov3-voc.cfg` — network definition
  - `yolov3-voc_9000.weights` — trained weights

## Build

1. Open `Detector/Detector/Detector.sln` in Visual Studio.
2. Point the project's **Include** and **Library** directories at your OpenCV
   `build/include` and `build/x64/vc15/lib` folders (the solution currently
   references a local OpenCV 3.4.3 path that you should update for your machine).
3. Select the **Release | x64** configuration and build.

## Configure

Edit `Detector/Detector/Configuration.cfg` (copied next to the executable at
run time):

| Key | Description | Example |
| --- | --- | --- |
| `NNCfg_Dir` | Folder containing the model files | `.\nncfg\` |
| `DetectedFrameDir` | Folder for violation snapshots | `.\result\` |
| `Interval` | Frames between detections (≈24 fps source) | `24` |
| `DInterval` | Frame interval after a detection | `24` |
| `Threshold` | Confidence threshold (0–1) | `0.9` |
| `DataSource` | SQL Server data source | `(local)` |
| `DB_Name` | Database name | `SecurityDetection` |
| `DB_User` | Database user | `sa` |
| `DB_Password` | Database password (**do not commit real secrets**) | `CHANGE_ME` |

## Run

```bat
:: detect on a single image
Detector.exe --image=Test\000200.jpg

:: detect on a video file
Detector.exe --video=video.mp4

:: detect on a live RTSP stream
Detector.exe --rtsp=rtsp://user:password@host:port/h264/ch1/main/av_stream
```

When a violation is detected the annotated frame is saved to `DetectedFrameDir`
and a row is inserted into the `DetectedRecord` table (created automatically on
first use) with the timestamp, alarm level, image name and confidence.

## Possible improvements / roadmap

- Cross-platform build (CMake) and a CI pipeline.
- GPU (CUDA) inference backend and upgrade to a newer detector (YOLOv5/v8).
- Pluggable storage backends and structured logging.
- Unit/integration tests around the configuration parser and the pipeline.

## License

Released under the [MIT License](LICENSE). Portions are based on the
YOLOv3-in-OpenCV object detection sample by BigVision LLC, distributed under the
OpenCV license.
