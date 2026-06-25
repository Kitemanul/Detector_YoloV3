# Changelog

All notable changes to this project are documented in this file.

## [Unreleased]

### Architecture overhaul
- Reorganized the flat `Detector/Detector/` tree into a modular `include/` +
  `src/` layout (`core`, `capture`, `inference`, `persistence`, `pipeline`).
- Added a cross-platform **CMake** build alongside the Visual Studio solution
  (which now references the new paths and no longer uses a precompiled header).
- Introduced a generic **`ThreadSafeQueue<T>`**: bounded, `condition_variable`
  -driven (no sleep-polling), with a `pushDropOldest` policy for live streams and
  a `close()`/drain protocol for clean end-of-stream propagation. Validated under
  ThreadSanitizer with multi-producer/consumer stress tests.
- Replaced the detached `while(1)` worker threads + scattered globals with a
  `Pipeline` orchestrator that owns the queues and **joins** its threads (RAII),
  with `SIGINT`-driven graceful shutdown and exception-safe worker loops.
- Fixed a major performance bug: the YOLOv3 network is now **loaded once** and
  reused, instead of being rebuilt from disk for every frame.
- Aligned detection results with NMS survivors (previously every pre-NMS box was
  reported, causing duplicate alarms).
- Decoupled storage behind an **`ISink`** interface: portable `FileSink`
  (snapshots + CSV) and Windows `SqlServerSink` (ADO), selectable at build time.
- Added a tiny thread-safe leveled logger and portable time utilities.

### Earlier cleanup
- Translated all in-code documentation from (mojibake) GBK Chinese to English.
- Replaced manual `mutex` lock/unlock with RAII `lock_guard` across the worker
  threads.
- Stopped the inference and database worker threads from busy-spinning the CPU
  when their queues are empty.
- Fixed self-assignment bugs in `NetResultDO` setters.
- Fixed `db_Operator` constructor failing to initialize its `userName` / `pwd`
  members.
- Simplified the capture loop (single read per iteration; no implicit camera-0
  fallback on open failure).
- Removed the committed database password; configuration now ships with a
  placeholder.
- Added `LICENSE`, `CHANGELOG.md`, `docs/ARCHITECTURE.md` and a rewritten
  `README.md`.

## [4.0]
- Reorganized the project structure.

## [3.4]
- Fixed a bounding-box offset bug.

## [3.3]
- Fixed an unresponsiveness bug during long-running RTSP playback.
- The database name can now be configured freely.

## [3.2]
- Reworked the threading model.
- Reworked frame iteration to fix unresponsiveness during long playback.

## [3.1]
- Updated the SQL Server connection string.
- Changed the snapshot naming scheme.
- Added `get_CurrentTime_s()`.

## [3.0]
- Added the `Configuration.cfg` configuration file.
- Added the database layer that records detected violations.
- Requires SQL Server 2017 + SSMS 2019 and `msado15.dll`.

## [2.0]
- Detects all four PPE combinations: helmet only (`10`), vest only (`01`),
  helmet + vest (`11`), and neither (`00`).
- `ProcessClass` now returns an alarm level instead of a boolean.

## [1.0]
- Initial C++ interface around a YOLOv3 detector.
- Supports image, video and RTSP inputs; samples one frame per second and saves
  a snapshot when a violation is detected.
