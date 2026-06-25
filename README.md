# PPE Safety Compliance Detector (YOLOv3 + C++/OpenCV)

A real-time **personal protective equipment (PPE) compliance detector** for
industrial/construction safety monitoring. It analyses an image, a video file,
or a live **RTSP** camera stream, detects whether each person is wearing a
**safety helmet** and a **reflective vest**, and automatically captures and
records every violation.

The application is written in modern C++17 on top of the **OpenCV DNN** module
(YOLOv3 / Darknet) and is built around a multithreaded **producer/consumer
pipeline** with bounded, condition-variable-driven queues, so real-time capture
is never blocked by inference or by storage I/O.

<p>
  <img alt="language" src="https://img.shields.io/badge/language-C%2B%2B17-00599C">
  <img alt="opencv" src="https://img.shields.io/badge/OpenCV-3.4%2B-5C3EE8">
  <img alt="model" src="https://img.shields.io/badge/model-YOLOv3-00FFFF">
  <img alt="build" src="https://img.shields.io/badge/build-CMake%20%7C%20VS2017-064F8C">
  <img alt="license" src="https://img.shields.io/badge/license-MIT-green">
</p>

---

## Highlights

- **Three-stage concurrent pipeline** (capture → inference → persistence). The
  stages are connected by a custom **bounded, thread-safe blocking queue**
  (`std::mutex` + two `std::condition_variable`s — no sleep-polling).
- **Real-time backpressure policy**: file inputs apply backpressure so no frame
  is lost; live/RTSP inputs **drop the oldest frame** under load so latency never
  grows.
- **Deterministic, graceful shutdown**: worker threads are RAII-joined (never
  detached); end-of-stream propagates by closing queues; `SIGINT` triggers a
  clean drain-and-join.
- **YOLOv3 detection** via OpenCV DNN with confidence filtering and NMS; the
  network is **loaded once** and reused for every frame.
- **Pluggable persistence** behind an `ISink` interface — a portable `FileSink`
  (snapshots + CSV) and a Windows `SqlServerSink` (ADO / SQL Server), selectable
  at build time. The core pipeline has no Windows or database dependency.
- **External configuration** via a simple `.cfg` file (thresholds, paths,
  sampling interval, DB credentials) — no recompilation needed.

## Architecture

```
 input (image/video/RTSP)
        │
        ▼
 ┌─────────────┐  Frame    ┌──────────────────┐  Detection  ┌─────────────────┐
 │  Capture    │ ────────► │  Inference       │ ──────────► │  Persistence    │
 │ (run thread)│  frameQ   │  DetectorNet 1×  │   resultQ   │  classify→ISink │
 └─────────────┘ (bounded) └──────────────────┘  (bounded)  └────────┬────────┘
                                                                      │
                                                        FileSink / SqlServerSink
```

See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for the concurrency model,
shutdown sequence, module layout, and the class → alarm-level mapping.

## Tech stack

| Area | Technology |
| --- | --- |
| Language | C++17 |
| Computer vision | OpenCV (DNN, imgproc, highgui, videoio) |
| Model | YOLOv3 (Darknet `.cfg` + `.weights`) |
| Concurrency | `std::thread`, `std::mutex`, `std::condition_variable`, `std::atomic` |
| Storage | Pluggable `ISink`: flat files (any OS) or SQL Server via ADO (`msado15`, Windows) |
| Build | CMake (cross-platform) **or** Visual Studio solution (`Detector.sln`) |

## Project structure

```
Detector_YoloV3/
├── CMakeLists.txt                  # cross-platform build
├── config/Configuration.cfg        # runtime configuration
├── include/detector/
│   ├── core/        Config.h  Log.h  Time.h  ThreadSafeQueue.h
│   ├── capture/     FrameSource.h
│   ├── inference/   DetectorNet.h  Frame.h  Detection.h
│   ├── persistence/ ISink.h  FileSink.h  SqlServerSink.h  AdoConnection.h
│   └── pipeline/    Pipeline.h
├── src/
│   ├── core/        Config.cpp  Log.cpp  Time.cpp
│   ├── capture/     FrameSource.cpp
│   ├── inference/   DetectorNet.cpp
│   ├── persistence/ FileSink.cpp  SqlServerSink.cpp  AdoConnection.cpp
│   └── pipeline/    Pipeline.cpp  main.cpp
├── Detector/Detector/Detector.sln  # Visual Studio solution (references ../../src + ../../include)
├── docs/ARCHITECTURE.md
├── CHANGELOG.md
└── LICENSE
```

## Prerequisites

- A **C++17** compiler (MSVC v141+/GCC/Clang).
- **OpenCV** (3.4+; the VS project links `opencv_world343*.lib`).
- **SQL Server** + `msado15.dll` — only for the optional `SqlServerSink`
  (Windows). The default `FileSink` needs nothing extra.
- YOLOv3 model files (not included), placed in `NNCfg_Dir`:
  - `voc.names` — class labels
  - `yolov3-voc.cfg` — network definition
  - `yolov3-voc_9000.weights` — trained weights

## Build

### CMake (Linux / macOS / Windows)

```bash
cmake -B build -DDETECTOR_WITH_SQLSERVER=OFF   # FileSink only (default off-Windows)
cmake --build build --config Release
```

`Configuration.cfg` is copied next to the binary automatically. Enable the SQL
Server backend on Windows with `-DDETECTOR_WITH_SQLSERVER=ON`.

### Visual Studio

1. Open `Detector/Detector/Detector.sln`.
2. Update the project's **Include**/**Library** directories to your OpenCV
   `build/include` and `build/x64/vc15/lib` (the solution ships a sample path).
3. Build **Release | x64** (the SQL Server sink is compiled in by default).

## Configure

Edit `config/Configuration.cfg` (copied next to the executable at run time):

| Key | Description | Example |
| --- | --- | --- |
| `NNCfg_Dir` | Folder containing the model files | `.\nncfg\` |
| `DetectedFrameDir` | Folder for violation snapshots / CSV | `.\result\` |
| `Interval` | Frames between sampled frames (≈24 fps source) | `24` |
| `Threshold` | Confidence threshold (0–1) | `0.9` |
| `DataSource` | SQL Server data source (SqlServerSink) | `(local)` |
| `DB_Name` | Database name | `SecurityDetection` |
| `DB_User` | Database user | `sa` |
| `DB_Password` | Database password (**do not commit real secrets**) | `CHANGE_ME` |

## Run

```bash
# detect on a single image
./Detector --image=Test/000200.jpg

# detect on a video file
./Detector --video=video.mp4

# detect on a live RTSP stream
./Detector --rtsp=rtsp://user:password@host:port/h264/ch1/main/av_stream
```

Press `Ctrl+C` for a graceful shutdown. When a violation is detected the
annotated frame is saved to `DetectedFrameDir`; the `FileSink` appends a row to
`detections.csv`, while the `SqlServerSink` inserts a row into the
`DetectedRecord` table (created automatically on first use).

## Roadmap

- GPU (CUDA) inference backend and an upgrade to a newer detector (YOLOv5/v8).
- Additional `ISink` backends (REST endpoint, message queue).
- A CI workflow building the cross-platform core and running the queue tests.

## License

Released under the [MIT License](LICENSE). Portions are based on the
YOLOv3-in-OpenCV object detection sample by BigVision LLC, distributed under the
OpenCV license.
