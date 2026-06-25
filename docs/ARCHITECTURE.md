# Architecture

The detector is a three-stage **producer/consumer pipeline**. Each stage runs on
its own thread and is connected to the next by a **bounded, thread-safe queue**.
This decouples real-time frame capture from the (slower) neural-network
inference and from storage I/O, so a slow database write can never stall the
video stream.

```
   image / video / RTSP
            │
 ┌──────────▼───────────┐   ThreadSafeQueue<Frame>    ┌───────────────────────┐
 │  Capture (run thread) │ ─────────  frameQ  ───────► │  Inference (thread)   │
 │  FrameSource + sample │   bounded · cond-var · drop │  DetectorNet (1×load) │
 └──────────────────────┘                             └───────────┬───────────┘
                                                                   │
                                              ThreadSafeQueue<Detection>
                                                          resultQ │ bounded · cond-var
                                                                   ▼
                                                       ┌───────────────────────┐
                                                       │  Persistence (thread) │
                                                       │  classify → ISink     │
                                                       └───────────┬───────────┘
                                                                   │
                                                        ┌──────────┴──────────┐
                                                        │  FileSink           │  (portable)
                                                        │  SqlServerSink      │  (Windows/ADO)
                                                        └─────────────────────┘
```

## Concurrency model

- **`ThreadSafeQueue<T>`** (`include/detector/core/ThreadSafeQueue.h`) — a bounded
  blocking queue backed by a `std::mutex` and two `std::condition_variable`s
  (`notEmpty`, `notFull`). No sleep-polling: producers/consumers block until
  there is work or space.
  - `push` applies **backpressure** (blocks when full) — used for files so no
    frame is dropped.
  - `pushDropOldest` **drops the oldest frame** when full — used for live/RTSP so
    latency never grows under load.
  - `close()` wakes all waiters; `waitPop` then drains the remainder and returns
    `false`, which is how end-of-stream propagates down the pipeline.
- **`Pipeline`** (`include/detector/pipeline/Pipeline.h`) owns the two queues and
  the worker `std::thread`s as members. Threads are **joined** (RAII), never
  detached. Shutdown is deterministic:
  `capture stops → frameQ.close() → inference drains → resultQ.close() →
  persistence drains → join all`.
- **Graceful stop**: a `SIGINT` handler calls `Pipeline::requestStop()`, which
  sets an `std::atomic<bool>` the capture loop polls. Worker exceptions (e.g. a
  missing model file) are caught, logged, and trigger the same clean shutdown.
- **Single model load**: the `DetectorNet` is constructed once in the inference
  thread and reused for every frame (the previous code rebuilt the network per
  frame).

## Module layout

| Module | Files | Responsibility |
| --- | --- | --- |
| `core` | `Config`, `Log`, `Time`, `ThreadSafeQueue` | Config parsing, leveled logging, time formatting, the queue primitive. |
| `capture` | `FrameSource` | Opens image/video/RTSP via `cv::VideoCapture`; flags live sources. |
| `inference` | `DetectorNet`, `Frame`, `Detection` | YOLOv3/OpenCV-DNN wrapper + the queue data objects. |
| `persistence` | `ISink`, `FileSink`, `SqlServerSink`, `AdoConnection` | Storage abstraction and its backends. |
| `pipeline` | `Pipeline`, `main` | Orchestration, CLI, sink selection, lifecycle. |

## Persistence abstraction

The pipeline depends only on **`ISink`** (`open` / `record` / `close`), so the
storage backend is pluggable:

- **`FileSink`** (default, cross-platform): writes the annotated snapshot as JPEG
  and appends a row to `detections.csv`.
- **`SqlServerSink`** (Windows only, compiled when `DETECTOR_WITH_SQLSERVER` is
  defined): writes the snapshot and inserts a row into the `DetectedRecord`
  table through `AdoConnection` (ADO / `msado15`).

Adding a new backend (e.g. a REST endpoint or message queue) means implementing
`ISink` and selecting it in `main.cpp` — no pipeline changes.

## Detection classes and alarm levels

`Pipeline::classifyAlarmLevel` maps a detected class id to an alarm level; level
`0` is compliant and is not recorded.

| Class id | Alarm level |
| --- | --- |
| 0 | 1 (most severe) |
| 1 | 2 |
| 2 | 3 |
| other | 0 (no record) |

The exact semantics depend on the label order in `voc.names` and the trained
model.
