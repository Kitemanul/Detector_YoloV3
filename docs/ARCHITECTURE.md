# Architecture

The detector is structured as a three-stage **producer/consumer pipeline**.
Each stage runs on its own thread and communicates with the next through a
mutex-protected queue. This decouples real-time frame capture from the
(comparatively slow) neural-network inference and from database/disk I/O, so a
slow database write can never stall the video stream.

```
                 ┌──────────────────────────────────────────────────────────┐
                 │                      main thread                           │
   image / video │  OpenInputFile()  ──►  read + sample 1 frame / interval    │
   / RTSP source │                          │                                 │
                 └──────────────────────────┼─────────────────────────────────┘
                                            │ push FrameDO
                                   ┌────────▼─────────┐
                                   │  Buffer (deque)  │  guarded by Thread_mutex
                                   └────────┬─────────┘
                                            │ pop
                 ┌──────────────────────────▼─────────────────────────────────┐
                 │              inference thread (ProcessFrame)                 │
                 │  DetectorNet::compute()  ──►  YOLOv3 forward + NMS           │
                 └──────────────────────────┬─────────────────────────────────┘
                                            │ push NetResultDO
                                   ┌────────▼─────────┐
                                   │ Buffer1 (deque)  │  guarded by Thread_mutex1
                                   └────────┬─────────┘
                                            │ pop
                 ┌──────────────────────────▼─────────────────────────────────┐
                 │               database thread (DBOperator)                  │
                 │  classify ► write snapshot (imwrite) ► INSERT (ADO/SQL)     │
                 └──────────────────────────────────────────────────────────┘
```

## Components

| File | Responsibility |
| --- | --- |
| `Detector.cpp` | Entry point. Parses CLI args, opens the input, runs the capture loop, spawns the worker threads. |
| `ProcessFrame.{h,cpp}` | Inference worker. Drains `Buffer`, runs `DetectorNet`, pushes results to `Buffer1`. |
| `DetectorNet.{h,cpp}` | OpenCV DNN wrapper around the YOLOv3 (Darknet) model: blob creation, forward pass, confidence filtering, non-maximum suppression, box drawing. |
| `DBOperator.{h,cpp}` | Database worker. Drains `Buffer1`, maps each detection to an alarm level, saves snapshots, records violations. |
| `db_Operator.{h,cpp}` | Low-level SQL Server access via ADO (`msado15`): connect, execute, insert, time formatting. |
| `Configuration.{h,cpp}` | Singleton `.cfg` reader (typed getters for string/int/float/bool). |
| `FrameDO.{h,cpp}` | Data object on the capture → inference queue (frame + timestamp). |
| `NetResultDO.{h,cpp}` | Data object on the inference → database queue (labels, confidences, timings, frame, timestamp). |

## Detection classes and alarm levels

The model emits one of four PPE states per detection. `DBOperator::ProcessClass`
maps the class id to an alarm level; level `0` means compliant and is not
recorded.

| Class id | Meaning | Alarm level |
| --- | --- | --- |
| 0 | Violation (most severe) | 1 |
| 1 | Violation | 2 |
| 2 | Violation | 3 |
| 3 | Compliant (helmet + vest) | 0 (no record) |

The exact semantics depend on the order of labels in `voc.names` and on the
trained model.

## Persistence

Violations are written to the `DetectedRecord` table (created automatically on
first insert) with the timestamp, snapshot directory, alarm level, image name
and confidence. The corresponding annotated frame is saved as a JPEG under the
configured `DetectedFrameDir`.
