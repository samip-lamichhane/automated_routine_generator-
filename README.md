# University Routine Generator & Timetable Scheduler

A C++ Qt application to manage instructors, courses, rooms, student batches and schedule class sessions with workload checks and JSON persistence.

## Key Features
- Manage entities (instructors, courses, rooms, batches)
- Automatic workload validation for instructors
- Dark-themed Qt GUI
- JSON-based data persistence

## Prerequisites
- C++17 compiler (GCC 13+, MinGW-w64)
- Qt 6+ with Widgets module

## Build & Run

### Console version
```bash
g++ main.cpp AppManager.cpp Course.cpp Instructor.cpp Student_batch.cpp classSession.cpp room.cpp timeslot.cpp -o generator.exe
./generator.exe
```

### Qt GUI version
```bash
# Ensure Qt and MinGW are in PATH
cd qt/qt
qmake
mingw32-make
./release/qt.exe   # after windeployqt if needed
```

## Data Persistence
Schedules and entity data are saved to `timetable_data.json` in the executable directory and loaded on startup.

## File Diagram

```text
project/
├── AppManager.cpp
├── AppManager.hpp
├── Course.cpp
├── Course.hpp
├── Instructor.cpp
├── Instructor.hpp
├── room.cpp
├── room.hpp
├── Student_batch.cpp
├── Student_batch.hpp
├── classSession.cpp
├── classSession.hpp
├── timeslot.cpp
├── timeslot.hpp
├── main.cpp
├── .gitignore
└── qt/
    ├── main.cpp
    ├── mainwindow.cpp
    ├── mainwindow.h
    ├── mainwindow.ui
    └── qt.pro
```

## External Dependencies

- **Qt 6+ (Widgets module)** – Provides the GUI framework and JSON handling.
- **C++17 compiler** (GCC 13+, MinGW-w64) – Required for modern language features.
- **Standard Library** – Used for core data structures and algorithms.
- **JSON persistence** – Implemented using Qt's `QJsonDocument`/`QJsonObject` classes (no third‑party JSON library needed).

These are the non‑basic tools and libraries you need to build and run the project.
