# University Routine Generator & Timetable Scheduler


## Key Features

*   **Comprehensive Entity Management:** Add and manage Instructors, Courses, Classrooms (Rooms), and Student Batches.
*   **Automatic Workload Checks:** The application automatically tracks weekly teaching hours for each instructor and rejects scheduling slots that exceed their hourly threshold.
*   **Interactive Scheduling:** Schedule class sessions by assigning a timeslot (day of week, start time, end time), an instructor, a course, a classroom, and a target student batch.
*   **Aesthetic Desktop Interface:** A beautiful, responsive dark-themed Qt GUI styled with custom Catppuccin Mocha-inspired aesthetics.
*   **Persistent Storage:** Auto-saves and auto-loads your configurations and schedules using JSON database storage (`timetable_data.json`) in the executable directory.
*   **Modular Architecture:** Written with a clean separation of concerns, decoupling model code (`AppManager`, `Instructor`, etc.) from the GUI code (`MainWindow`).


## Directory Structure

```text
├── AppManager.cpp / .hpp      # Controller: Manages global timetable state and lookups
├── Course.cpp / .hpp          # Model: Course details and allocated lecture hours
├── Instructor.cpp / .hpp      # Model: Instructor details and weekly workload checks
├── room.cpp / .hpp            # Model: Classroom identifiers, capacities, and room types
├── Student_batch.cpp / .hpp   # Model: Batch IDs, student strengths, and program types
├── classSession.cpp / .hpp    # Model: Holds a scheduled course slot representation
├── timeslot.cpp / .hpp        # Model: Encapsulates weekdays, start and end clock times
├── main.cpp                   # Entry point for the Console-only test environment
├── .gitignore                 # Excludes build and compiler artifacts
└── qt/
    └── qt/
        ├── main.cpp           # Entry point for the Qt GUI application
        ├── mainwindow.cpp     # Controls UI forms, tables, styling, and JSON persistence
        ├── mainwindow.h       # UI definitions and signal/slot bindings
        ├── mainwindow.ui      # Default XML UI layout file (configured dynamically)
        └── qt.pro             # Qt project configuration file
```


## Prerequisites

*   **C++ Compiler:** A compiler supporting C++17 (e.g., GCC 13+ or MinGW-w64).
*   **Qt SDK:** Qt 6.0 or higher with the **Widgets** module.

## Build & Run Instructions

### 1. Build and Run the Console Version (Raw C++)

To test the basic model integration in a command prompt or shell:

```bash
# Compile the console app
g++ main.cpp AppManager.cpp Course.cpp Instructor.cpp Student_batch.cpp classSession.cpp room.cpp timeslot.cpp -o generator.exe

# Run the executable
./generator.exe
```

### 2. Build and Run the Qt GUI Desktop Version

You can build the desktop app using Qt Creator or directly from a MinGW-w64/Qt command line:

```bash
# 1. Add Qt tools and MinGW compiler to your system PATH
# (Replace paths with your local installation coordinates)
export PATH="/c/Qt/Tools/mingw1310_64/bin:/c/Qt/6.11.1/mingw_64/bin:$PATH"

# 2. Navigate to the Qt project directory
cd qt/qt

# 3. Generate Makefiles
qmake

# 4. Compile the project
mingw32-make
```

To run the application standalone on Windows, deploy the required Qt dependencies into the output folder:

```bash
# Deploy DLL runtimes into the release output folder
windeployqt release/qt.exe

# Run the application
./release/qt.exe
```

---

## JSON Data Persistence

Timetable routines and master lists are persisted inside a file named `timetable_data.json` located next to your `qt.exe` binary. 

*   On first launch, if no file is found, the system seeds default mockup entries automatically.
*   All additions (Instructors, Courses, Rooms, Student Batches, or Scheduled Slots) are saved immediately.
*   You can copy or share this JSON file to move schedules between workspaces.


qt.md : 
# Qt Module Documentation (Supervisor Q&A)

## 1. What is implemented in the Qt part of this project?
The Qt module is a full desktop GUI wrapper for the timetable engine implemented in the core C++ classes (`AppManager`, `Instructor`, `Course`, `Room`, `StudentBatch`, `ClassSession`, `TimeSlot`).

It allows the user to:
- Add instructors
- Add courses
- Add rooms
- Add student batches
- Schedule class sessions
- View timetable in table format
- Persist and reload all data from JSON

## 2. Which files make up the Qt implementation?
- `qt/main.cpp`: Qt application entry point (`QApplication`, create/show `MainWindow`).
- `qt/mainwindow.h`: Main window class declaration, slots, and UI component pointers.
- `qt/mainwindow.cpp`: Actual UI construction, signal-slot wiring, business flow integration, JSON save/load.
- `qt/mainwindow.ui`: Minimal Designer file (base `QMainWindow` scaffold).
- `qt/qt.pro`: qmake project file including Qt Widgets and all core source/header files from parent directory.

## 3. Why is the `.ui` file minimal if the UI is complex?
The project uses `ui->setupUi(this)` to initialize the base window from Designer, but then builds almost all widgets programmatically in `setupUI()`. This gives better control over dynamic tab creation and custom layout logic.

## 4. What architecture is used?
A practical layered approach:
- Presentation layer: Qt Widgets (`MainWindow`)
- Domain layer: Existing C++ classes (entity and scheduling logic)
- Persistence layer: JSON serialization/deserialization in `MainWindow`

The GUI does not duplicate domain logic; it delegates to `AppManager` and related classes.

## 5. How does app startup work?
1. `main.cpp` creates `QApplication`.
2. `MainWindow` constructor runs.
3. Dark stylesheet is applied.
4. `setupUI()` creates all tabs/forms/lists/table.
5. `loadFromFile()` loads saved data, or seeds defaults if no file exists/corrupted.
6. `refreshListsAndTables()` renders timetable.
7. `populateCombos()` updates scheduling dropdowns.

## 6. What tabs are implemented?
`QTabWidget` contains 5 tabs:
1. Instructors
2. Courses
3. Rooms
4. Student Batches
5. Generate & View Timetable

Each input tab has:
- Left side: form + Add button
- Right side: list view of created entities

Timetable tab has:
- Left side: session scheduling form
- Right side: `QTableWidget` showing scheduled sessions

## 7. What validations are implemented?
### Generic input validation
- Empty text checks for instructor/course/room/batch IDs.

### Scheduling validation
- All required dropdown entities must exist and be selected.
- Retrieved pointers (instructor/course/room/batch) must be valid.
- End time must be strictly greater than start time.
- Instructor workload guard via `assignNewCourse()`:
  - If assigning course exceeds max weekly hours, scheduling is blocked.
  - User sees detailed warning with max, current, and requested hours.

## 8. How is timetable data displayed?
`refreshListsAndTables()` clears and repopulates `QTableWidget` rows from `m_appManager.getTimetable()`.

Columns displayed:
- Day
- Time (formatted `HH:MM - HH:MM`)
- Course
- Instructor
- Room
- Batch
- Duration (minutes)

## 9. How is data persistence handled?
JSON file: `timetable_data.json` in `QCoreApplication::applicationDirPath()`.

### `saveToFile()` stores
- Instructors (+ assigned course codes)
- Courses
- Rooms
- Batches
- Timetable sessions (day/time + linked entity identifiers)

### `loadFromFile()` restores
Load order is intentional to resolve references safely:
1. Courses
2. Instructors (re-assign saved courses)
3. Rooms
4. Batches
5. Timetable sessions

If file is missing/unreadable/invalid JSON:
- Calls `populateInitialData()`
- Immediately writes fresh JSON via `saveToFile()`

## 10. Why does loading order matter?
Class sessions reference instructor/course/room/batch objects. These entities must already exist before sessions are recreated. Also instructors restore assigned courses, so courses are loaded first.

## 11. How are combos synchronized with data?
`populateCombos()` repopulates scheduling dropdowns by parsing current list widget text and extracting IDs/names before metadata suffixes.

Called after every add action so scheduler options stay up to date.

## 12. What user feedback mechanisms are used?
Qt message boxes:
- `QMessageBox::warning` for input/time/workload issues
- `QMessageBox::critical` for internal lookup failure
- `QMessageBox::information` for successful scheduling

This makes validation outcomes explicit during demo.

## 13. What styling/customization is implemented?
A custom dark stylesheet (Catppuccin-inspired) is applied globally in constructor for:
- Main window background
- Tab styles
- Form fields
- Buttons
- List/table widgets
- Header sections

Result: consistent modern visual theme without external QSS files.

## 14. How does Qt integrate with existing non-Qt code?
`qt.pro` includes all core `.cpp/.hpp` files from parent directory (`../../...`). This reuses your existing scheduling/business classes directly in the GUI build.

## 15. What are current limitations (honest review answer)?
- No conflict checking for room-time overlap or instructor-time overlap is visible in UI layer.
- No edit/delete actions for existing entities/sessions.
- `populateCombos()` depends on parsing list text format (works now, but coupling to display strings is fragile).
- Persistence is file-based JSON only; no DB/backend.
- Error handling is functional but not deeply granular (for example, no schema versioning for JSON).

## 16. Why this implementation is acceptable for project scope?
- Demonstrates complete GUI CRUD flow for core entities.
- Demonstrates scheduling with key academic constraint (instructor max workload).
- Demonstrates durable persistence across app restarts.
- Demonstrates integration of OOP domain model with event-driven Qt UI.
- Clean enough to extend with conflict checks and edit/delete features.

## 17. If supervisor asks "what exactly did you build personally in Qt?"
Suggested answer:
"I built the full Qt Widgets front-end around our existing timetable engine. I implemented the tab-based UI, all signal-slot handlers for adding entities and scheduling classes, JSON save/load with reconstruction logic, runtime validations including instructor workload checks, and timetable visualization in a table. I also integrated the legacy C++ domain classes into qmake and added a consistent custom dark theme for usability."

## 18. Quick viva defense points
- "UI is programmatic for dynamic control and maintainability of complex tab logic."
- "Persistence strategy is deterministic: save full state, restore in dependency-safe order."
- "Business constraints are enforced before state mutation, not only after display."
- "Qt layer remains thin; domain logic lives in reusable core classes."
- "Current design is intentionally extensible for conflict-detection and editing workflows."
