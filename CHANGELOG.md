# Changelog

All notable changes to this project are documented in this file.

## [Unreleased]
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
