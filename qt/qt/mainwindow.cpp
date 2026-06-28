#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Instructor.hpp"
#include "Course.hpp"
#include "room.hpp"
#include "Student_batch.hpp"
#include "classSession.hpp"
#include "timeslot.hpp"
#include "AppManager.hpp"

#include <QHeaderView>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>

// Helper to convert Day enum to QString
static QString dayToString(Day d) {
    switch(d) {
        case Day::Monday:    return "Monday";
        case Day::Tuesday:   return "Tuesday";
        case Day::Wednesday: return "Wednesday";
        case Day::Thursday:  return "Thursday";
        case Day::Friday:    return "Friday";
        default:             return "Unknown";
    }
}

// Helper to format ClockTime as HH:MM
static QString formatClockTime(ClockTime t) {
    return QString("%1:%2")
        .arg(t.hours, 2, 10, QChar('0'))
        .arg(t.minutes, 2, 10, QChar('0'));
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    // Set modern Catppuccin-inspired dark stylesheet
    this->setStyleSheet(R"(
        QMainWindow {
            background-color: #1e1e2e;
        }
        QTabWidget::pane {
            border: 1px solid #313244;
            background-color: #181825;
            border-radius: 8px;
        }
        QTabBar::tab {
            background: #11111b;
            color: #cdd6f4;
            border: 1px solid #313244;
            padding: 10px 20px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            font-weight: bold;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QTabBar::tab:selected, QTabBar::tab:hover {
            background: #181825;
            border-bottom-color: #181825;
            color: #89b4fa;
        }
        QLabel {
            color: #cdd6f4;
            font-size: 13px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
            font-weight: 500;
        }
        QLineEdit, QSpinBox, QComboBox, QTimeEdit {
            background-color: #313244;
            color: #cdd6f4;
            border: 1px solid #45475a;
            border-radius: 6px;
            padding: 6px;
            font-size: 13px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QTimeEdit:focus {
            border: 1px solid #89b4fa;
        }
        QPushButton {
            background-color: #89b4fa;
            color: #11111b;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
            font-size: 13px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QPushButton:hover {
            background-color: #b4befe;
        }
        QPushButton:pressed {
            background-color: #74c7ec;
        }
        QListWidget, QTableWidget {
            background-color: #11111b;
            color: #cdd6f4;
            border: 1px solid #313244;
            border-radius: 8px;
            padding: 5px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QHeaderView::section {
            background-color: #313244;
            color: #cdd6f4;
            border: 1px solid #45475a;
            padding: 6px;
            font-weight: bold;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QTableWidget QTableCornerButton::section {
            background-color: #313244;
        }
    )");

    setupUI();
    loadFromFile();
    refreshListsAndTables();
    populateCombos();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUI()
{
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    // ==========================================
    // 1. INSTRUCTORS TAB
    // ==========================================
    QWidget *instTab = new QWidget();
    QHBoxLayout *instLayout = new QHBoxLayout(instTab);
    
    // Left Form
    QWidget *instLeft = new QWidget();
    QVBoxLayout *instFormLayout = new QVBoxLayout(instLeft);
    QFormLayout *instForm = new QFormLayout();
    
    m_instNameEdit = new QLineEdit();
    m_instHoursSpin = new QSpinBox();
    m_instHoursSpin->setRange(1, 100);
    m_instHoursSpin->setValue(20);
    
    instForm->addRow(new QLabel("Instructor Name:"), m_instNameEdit);
    instForm->addRow(new QLabel("Max Weekly Hours:"), m_instHoursSpin);
    
    QPushButton *btnInstAdd = new QPushButton("Add Instructor");
    connect(btnInstAdd, &QPushButton::clicked, this, &MainWindow::onAddInstructor);
    
    instFormLayout->addLayout(instForm);
    instFormLayout->addWidget(btnInstAdd);
    instFormLayout->addStretch();
    
    // Right List
    m_instList = new QListWidget();
    
    instLayout->addWidget(instLeft, 1);
    instLayout->addWidget(m_instList, 2);
    m_tabWidget->addTab(instTab, "Instructors");

    // ==========================================
    // 2. COURSES TAB
    // ==========================================
    QWidget *courseTab = new QWidget();
    QHBoxLayout *courseLayout = new QHBoxLayout(courseTab);
    
    // Left Form
    QWidget *courseLeft = new QWidget();
    QVBoxLayout *courseFormLayout = new QVBoxLayout(courseLeft);
    QFormLayout *courseForm = new QFormLayout();
    
    m_courseCodeEdit = new QLineEdit();
    m_courseHoursSpin = new QSpinBox();
    m_courseHoursSpin->setRange(1, 20);
    m_courseHoursSpin->setValue(3);
    
    courseForm->addRow(new QLabel("Course Code:"), m_courseCodeEdit);
    courseForm->addRow(new QLabel("Allocated Hours:"), m_courseHoursSpin);
    
    QPushButton *btnCourseAdd = new QPushButton("Add Course");
    connect(btnCourseAdd, &QPushButton::clicked, this, &MainWindow::onAddCourse);
    
    courseFormLayout->addLayout(courseForm);
    courseFormLayout->addWidget(btnCourseAdd);
    courseFormLayout->addStretch();
    
    // Right List
    m_courseList = new QListWidget();
    
    courseLayout->addWidget(courseLeft, 1);
    courseLayout->addWidget(m_courseList, 2);
    m_tabWidget->addTab(courseTab, "Courses");

    // ==========================================
    // 3. ROOMS TAB
    // ==========================================
    QWidget *roomTab = new QWidget();
    QHBoxLayout *roomLayout = new QHBoxLayout(roomTab);
    
    // Left Form
    QWidget *roomLeft = new QWidget();
    QVBoxLayout *roomFormLayout = new QVBoxLayout(roomLeft);
    QFormLayout *roomForm = new QFormLayout();
    
    m_roomIdEdit = new QLineEdit();
    m_roomCapSpin = new QSpinBox();
    m_roomCapSpin->setRange(1, 500);
    m_roomCapSpin->setValue(60);
    m_roomTypeCombo = new QComboBox();
    m_roomTypeCombo->addItems({"Theory", "Lab", "Auditorium"});
    
    roomForm->addRow(new QLabel("Room ID/Name:"), m_roomIdEdit);
    roomForm->addRow(new QLabel("Capacity:"), m_roomCapSpin);
    roomForm->addRow(new QLabel("Room Type:"), m_roomTypeCombo);
    
    QPushButton *btnRoomAdd = new QPushButton("Add Room");
    connect(btnRoomAdd, &QPushButton::clicked, this, &MainWindow::onAddRoom);
    
    roomFormLayout->addLayout(roomForm);
    roomFormLayout->addWidget(btnRoomAdd);
    roomFormLayout->addStretch();
    
    // Right List
    m_roomList = new QListWidget();
    
    roomLayout->addWidget(roomLeft, 1);
    roomLayout->addWidget(m_roomList, 2);
    m_tabWidget->addTab(roomTab, "Rooms");

    // ==========================================
    // 4. STUDENT BATCHES TAB
    // ==========================================
    QWidget *batchTab = new QWidget();
    QHBoxLayout *batchLayout = new QHBoxLayout(batchTab);
    
    // Left Form
    QWidget *batchLeft = new QWidget();
    QVBoxLayout *batchFormLayout = new QVBoxLayout(batchLeft);
    QFormLayout *batchForm = new QFormLayout();
    
    m_batchIdEdit = new QLineEdit();
    m_batchStrengthSpin = new QSpinBox();
    m_batchStrengthSpin->setRange(1, 200);
    m_batchStrengthSpin->setValue(45);
    m_batchProgCombo = new QComboBox();
    m_batchProgCombo->addItems({"BIT", "BCE", "BCS"});
    
    batchForm->addRow(new QLabel("Batch ID/Name:"), m_batchIdEdit);
    batchForm->addRow(new QLabel("Strength:"), m_batchStrengthSpin);
    batchForm->addRow(new QLabel("Program:"), m_batchProgCombo);
    
    QPushButton *btnBatchAdd = new QPushButton("Add Batch");
    connect(btnBatchAdd, &QPushButton::clicked, this, &MainWindow::onAddBatch);
    
    batchFormLayout->addLayout(batchForm);
    batchFormLayout->addWidget(btnBatchAdd);
    batchFormLayout->addStretch();
    
    // Right List
    m_batchList = new QListWidget();
    
    batchLayout->addWidget(batchLeft, 1);
    batchLayout->addWidget(m_batchList, 2);
    m_tabWidget->addTab(batchTab, "Student Batches");

    // ==========================================
    // 5. TIMETABLE / SCHEDULER TAB
    // ==========================================
    QWidget *timetableTab = new QWidget();
    QHBoxLayout *timetableLayout = new QHBoxLayout(timetableTab);
    
    // Left Form
    QWidget *timetableLeft = new QWidget();
    QVBoxLayout *timetableFormLayout = new QVBoxLayout(timetableLeft);
    QFormLayout *sessionForm = new QFormLayout();
    
    m_sessInstCombo = new QComboBox();
    m_sessCourseCombo = new QComboBox();
    m_sessRoomCombo = new QComboBox();
    m_sessBatchCombo = new QComboBox();
    
    m_sessDayCombo = new QComboBox();
    m_sessDayCombo->addItems({"Monday", "Tuesday", "Wednesday", "Thursday", "Friday"});
    
    m_sessStartEdit = new QTimeEdit(QTime(9, 30));
    m_sessEndEdit = new QTimeEdit(QTime(11, 30));
    
    sessionForm->addRow(new QLabel("Instructor:"), m_sessInstCombo);
    sessionForm->addRow(new QLabel("Course:"), m_sessCourseCombo);
    sessionForm->addRow(new QLabel("Room:"), m_sessRoomCombo);
    sessionForm->addRow(new QLabel("Student Batch:"), m_sessBatchCombo);
    sessionForm->addRow(new QLabel("Day:"), m_sessDayCombo);
    sessionForm->addRow(new QLabel("Start Time:"), m_sessStartEdit);
    sessionForm->addRow(new QLabel("End Time:"), m_sessEndEdit);
    
    QPushButton *btnSessionAdd = new QPushButton("Schedule Class Session");
    connect(btnSessionAdd, &QPushButton::clicked, this, &MainWindow::onAddClassSession);
    
    timetableFormLayout->addLayout(sessionForm);
    timetableFormLayout->addWidget(btnSessionAdd);
    timetableFormLayout->addStretch();
    
    // Right Table
    m_timetableTable = new QTableWidget();
    m_timetableTable->setColumnCount(7);
    m_timetableTable->setHorizontalHeaderLabels({"Day", "Time", "Course", "Instructor", "Room", "Batch", "Duration"});
    m_timetableTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_timetableTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    timetableLayout->addWidget(timetableLeft, 1);
    timetableLayout->addWidget(m_timetableTable, 2);
    m_tabWidget->addTab(timetableTab, "Generate & View Timetable");
}

void MainWindow::populateInitialData()
{
    // Populate some default data matching console main.cpp to make tests easy
    m_appManager.addInstructor(Instructor("Dr. Niraj Sharma", 12));
    m_appManager.addInstructor(Instructor("Prof. Ram Prasad", 15));
    
    m_appManager.addCourse(Course("COMP-102", 3));
    m_appManager.addCourse(Course("MATH-101", 4));
    
    m_appManager.addRoom(Room("Block-C-102", 60, RoomType::Theory));
    m_appManager.addRoom(Room("Lab-A-301", 40, RoomType::Lab));
    
    m_appManager.addBatch(StudentBatch("BCT-2025-A", 48, ProgramType::BCE));
    m_appManager.addBatch(StudentBatch("BIT-2025-B", 45, ProgramType::BIT));

    m_instList->addItem("Dr. Niraj Sharma (Max Hours: 12)");
    m_instList->addItem("Prof. Ram Prasad (Max Hours: 15)");
    
    m_courseList->addItem("COMP-102 (Allocated Hours: 3)");
    m_courseList->addItem("MATH-101 (Allocated Hours: 4)");
    
    m_roomList->addItem("Block-C-102 (Capacity: 60, Type: Theory)");
    m_roomList->addItem("Lab-A-301 (Capacity: 40, Type: Lab)");
    
    m_batchList->addItem("BCT-2025-A (Strength: 48, Program: BCE)");
    m_batchList->addItem("BIT-2025-B (Strength: 45, Program: BIT)");
}

void MainWindow::saveToFile()
{
    QString filePath = QCoreApplication::applicationDirPath() + "/timetable_data.json";
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }

    QJsonObject rootObj;

    // 1. Instructors
    QJsonArray instArray;
    for (const auto& inst : m_appManager.getInstructors()) {
        QJsonObject instObj;
        instObj["name"] = QString::fromStdString(inst.getName());
        instObj["maxLimitHours"] = inst.getMaxLimitHours();
        
        QJsonArray assignedArray;
        for (const auto& crs : inst.getAssignedCourses()) {
            assignedArray.append(QString::fromStdString(crs.getCourseCode()));
        }
        instObj["assignedCourses"] = assignedArray;
        instArray.append(instObj);
    }
    rootObj["instructors"] = instArray;

    // 2. Courses
    QJsonArray courseArray;
    for (const auto& crs : m_appManager.getCourses()) {
        QJsonObject crsObj;
        crsObj["courseCode"] = QString::fromStdString(crs.getCourseCode());
        crsObj["allocatedHours"] = crs.getAllocatedHours();
        courseArray.append(crsObj);
    }
    rootObj["courses"] = courseArray;

    // 3. Rooms
    QJsonArray roomArray;
    for (const auto& rm : m_appManager.getRooms()) {
        QJsonObject rmObj;
        rmObj["roomId"] = QString::fromStdString(rm.getRoomId());
        rmObj["capacity"] = rm.getCapacity();
        rmObj["type"] = static_cast<int>(rm.getType());
        roomArray.append(rmObj);
    }
    rootObj["rooms"] = roomArray;

    // 4. Batches
    QJsonArray batchArray;
    for (const auto& b : m_appManager.getBatches()) {
        QJsonObject bObj;
        bObj["batchId"] = QString::fromStdString(b.getBatchId());
        bObj["strength"] = b.getStrength();
        bObj["program"] = static_cast<int>(b.getProgram());
        batchArray.append(bObj);
    }
    rootObj["batches"] = batchArray;

    // 5. Timetable (Class Sessions)
    QJsonArray sessionArray;
    for (const auto& s : m_appManager.getTimetable()) {
        QJsonObject sObj;
        TimeSlot ts = s.getTimeSlot();
        sObj["day"] = static_cast<int>(ts.getDay());
        sObj["startH"] = ts.getStartTime().hours;
        sObj["startM"] = ts.getStartTime().minutes;
        sObj["endH"] = ts.getEndTime().hours;
        sObj["endM"] = ts.getEndTime().minutes;
        sObj["instructor"] = QString::fromStdString(s.getTeacherId()->getName());
        sObj["course"] = QString::fromStdString(s.getSubjectId()->getCourseCode());
        sObj["room"] = QString::fromStdString(s.getRoomId()->getRoomId());
        sObj["batch"] = QString::fromStdString(s.getBatchId()->getBatchId());
        sessionArray.append(sObj);
    }
    rootObj["timetable"] = sessionArray;

    QJsonDocument doc(rootObj);
    file.write(doc.toJson());
    file.close();
}

void MainWindow::loadFromFile()
{
    QString filePath = QCoreApplication::applicationDirPath() + "/timetable_data.json";
    if (!QFile::exists(filePath)) {
        populateInitialData();
        saveToFile();
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        populateInitialData();
        saveToFile();
        return;
    }

    QByteArray saveData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(saveData);
    if (doc.isNull() || !doc.isObject()) {
        populateInitialData();
        saveToFile();
        return;
    }

    QJsonObject rootObj = doc.object();

    // 1. Courses (Load first as they are needed by Instructors and Class Sessions)
    if (rootObj.contains("courses") && rootObj["courses"].isArray()) {
        QJsonArray courseArray = rootObj["courses"].toArray();
        for (const auto& val : courseArray) {
            QJsonObject crsObj = val.toObject();
            std::string code = crsObj["courseCode"].toString().toStdString();
            int hours = crsObj["allocatedHours"].toInt();
            m_appManager.addCourse(Course(code, hours));
        }
    }

    // 2. Instructors
    if (rootObj.contains("instructors") && rootObj["instructors"].isArray()) {
        QJsonArray instArray = rootObj["instructors"].toArray();
        for (const auto& val : instArray) {
            QJsonObject instObj = val.toObject();
            std::string name = instObj["name"].toString().toStdString();
            int maxHours = instObj["maxLimitHours"].toInt();
            Instructor inst(name, maxHours);
            
            // Re-assign courses
            if (instObj.contains("assignedCourses") && instObj["assignedCourses"].isArray()) {
                QJsonArray assignedArray = instObj["assignedCourses"].toArray();
                for (const auto& cVal : assignedArray) {
                    std::string courseCode = cVal.toString().toStdString();
                    Course* crs = m_appManager.findCourseByCode(courseCode);
                    if (crs) {
                        inst.assignNewCourse(*crs);
                    }
                }
            }
            m_appManager.addInstructor(inst);
        }
    }

    // 3. Rooms
    if (rootObj.contains("rooms") && rootObj["rooms"].isArray()) {
        QJsonArray roomArray = rootObj["rooms"].toArray();
        for (const auto& val : roomArray) {
            QJsonObject rmObj = val.toObject();
            std::string id = rmObj["roomId"].toString().toStdString();
            int cap = rmObj["capacity"].toInt();
            RoomType type = static_cast<RoomType>(rmObj["type"].toInt());
            m_appManager.addRoom(Room(id, cap, type));
        }
    }

    // 4. Batches
    if (rootObj.contains("batches") && rootObj["batches"].isArray()) {
        QJsonArray batchArray = rootObj["batches"].toArray();
        for (const auto& val : batchArray) {
            QJsonObject bObj = val.toObject();
            std::string id = bObj["batchId"].toString().toStdString();
            int strength = bObj["strength"].toInt();
            ProgramType prog = static_cast<ProgramType>(bObj["program"].toInt());
            m_appManager.addBatch(StudentBatch(id, strength, prog));
        }
    }

    // 5. Timetable (Class Sessions)
    if (rootObj.contains("timetable") && rootObj["timetable"].isArray()) {
        QJsonArray sessionArray = rootObj["timetable"].toArray();
        for (const auto& val : sessionArray) {
            QJsonObject sObj = val.toObject();
            Day day = static_cast<Day>(sObj["day"].toInt());
            ClockTime ctStart{ sObj["startH"].toInt(), sObj["startM"].toInt() };
            ClockTime ctEnd{ sObj["endH"].toInt(), sObj["endM"].toInt() };
            TimeSlot slot(day, ctStart, ctEnd);

            std::string instName = sObj["instructor"].toString().toStdString();
            std::string courseCode = sObj["course"].toString().toStdString();
            std::string roomId = sObj["room"].toString().toStdString();
            std::string batchId = sObj["batch"].toString().toStdString();

            Instructor* inst = m_appManager.findInstructorByName(instName);
            Course* crs = m_appManager.findCourseByCode(courseCode);
            Room* rm = m_appManager.findRoomById(roomId);
            StudentBatch* btch = m_appManager.findBatchById(batchId);

            if (inst && crs && rm && btch) {
                m_appManager.addClassSession(ClassSession(slot, inst, crs, rm, btch));
            }
        }
    }

    // Populate list widgets based on loaded data
    for (const auto& inst : m_appManager.getInstructors()) {
        m_instList->addItem(QString("%1 (Max Hours: %2)")
            .arg(QString::fromStdString(inst.getName()))
            .arg(inst.getMaxLimitHours()));
    }
    for (const auto& crs : m_appManager.getCourses()) {
        m_courseList->addItem(QString("%1 (Allocated Hours: %2)")
            .arg(QString::fromStdString(crs.getCourseCode()))
            .arg(crs.getAllocatedHours()));
    }
    for (const auto& rm : m_appManager.getRooms()) {
        m_roomList->addItem(QString("%1 (Capacity: %2, Type: %3)")
            .arg(QString::fromStdString(rm.getRoomId()))
            .arg(rm.getCapacity())
            .arg(QString::fromStdString(rm.getTypeAsString())));
    }
    for (const auto& b : m_appManager.getBatches()) {
        m_batchList->addItem(QString("%1 (Strength: %2, Program: %3)")
            .arg(QString::fromStdString(b.getBatchId()))
            .arg(b.getStrength())
            .arg(QString::fromStdString(b.getProgramAsString())));
    }
}

void MainWindow::populateCombos()
{
    m_sessInstCombo->clear();
    m_sessCourseCombo->clear();
    m_sessRoomCombo->clear();
    m_sessBatchCombo->clear();

    for(int i = 0; i < m_instList->count(); ++i) {
        QString text = m_instList->item(i)->text();
        int idx = text.indexOf(" (Max Hours:");
        if (idx != -1) {
            m_sessInstCombo->addItem(text.left(idx));
        }
    }

    for(int i = 0; i < m_courseList->count(); ++i) {
        QString text = m_courseList->item(i)->text();
        int idx = text.indexOf(" (Allocated Hours:");
        if (idx != -1) {
            m_sessCourseCombo->addItem(text.left(idx));
        }
    }

    for(int i = 0; i < m_roomList->count(); ++i) {
        QString text = m_roomList->item(i)->text();
        int idx = text.indexOf(" (Capacity:");
        if (idx != -1) {
            m_sessRoomCombo->addItem(text.left(idx));
        }
    }

    for(int i = 0; i < m_batchList->count(); ++i) {
        QString text = m_batchList->item(i)->text();
        int idx = text.indexOf(" (Strength:");
        if (idx != -1) {
            m_sessBatchCombo->addItem(text.left(idx));
        }
    }
}

void MainWindow::refreshListsAndTables()
{
    // Refresh Timetable Table
    const auto& timetable = m_appManager.getTimetable();
    m_timetableTable->setRowCount(0);
    for (const auto& session : timetable) {
        int row = m_timetableTable->rowCount();
        m_timetableTable->insertRow(row);
        
        TimeSlot ts = session.getTimeSlot();
        QString timeStr = formatClockTime(ts.getStartTime()) + " - " + formatClockTime(ts.getEndTime());
        
        m_timetableTable->setItem(row, 0, new QTableWidgetItem(dayToString(ts.getDay())));
        m_timetableTable->setItem(row, 1, new QTableWidgetItem(timeStr));
        m_timetableTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(session.getSubjectId()->getCourseCode())));
        m_timetableTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(session.getTeacherId()->getName())));
        m_timetableTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(session.getRoomId()->getRoomId())));
        m_timetableTable->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(session.getBatchId()->getBatchId())));
        m_timetableTable->setItem(row, 6, new QTableWidgetItem(QString("%1 mins").arg(ts.getDurationmin())));
    }
}

void MainWindow::onAddInstructor()
{
    QString name = m_instNameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Instructor name cannot be empty.");
        return;
    }
    
    int hours = m_instHoursSpin->value();
    m_appManager.addInstructor(Instructor(name.toStdString(), hours));
    
    m_instList->addItem(QString("%1 (Max Hours: %2)").arg(name).arg(hours));
    m_instNameEdit->clear();
    
    saveToFile();
    populateCombos();
}

void MainWindow::onAddCourse()
{
    QString code = m_courseCodeEdit->text().trimmed();
    if (code.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Course code cannot be empty.");
        return;
    }
    
    int hours = m_courseHoursSpin->value();
    m_appManager.addCourse(Course(code.toStdString(), hours));
    
    m_courseList->addItem(QString("%1 (Allocated Hours: %2)").arg(code).arg(hours));
    m_courseCodeEdit->clear();
    
    saveToFile();
    populateCombos();
}

void MainWindow::onAddRoom()
{
    QString id = m_roomIdEdit->text().trimmed();
    if (id.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Room ID cannot be empty.");
        return;
    }
    
    int cap = m_roomCapSpin->value();
    RoomType type = static_cast<RoomType>(m_roomTypeCombo->currentIndex());
    QString typeStr = m_roomTypeCombo->currentText();
    
    m_appManager.addRoom(Room(id.toStdString(), cap, type));
    
    m_roomList->addItem(QString("%1 (Capacity: %2, Type: %3)").arg(id).arg(cap).arg(typeStr));
    m_roomIdEdit->clear();
    
    saveToFile();
    populateCombos();
}

void MainWindow::onAddBatch()
{
    QString id = m_batchIdEdit->text().trimmed();
    if (id.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Batch ID cannot be empty.");
        return;
    }
    
    int strength = m_batchStrengthSpin->value();
    ProgramType prog = static_cast<ProgramType>(m_batchProgCombo->currentIndex());
    QString progStr = m_batchProgCombo->currentText();
    
    m_appManager.addBatch(StudentBatch(id.toStdString(), strength, prog));
    
    m_batchList->addItem(QString("%1 (Strength: %2, Program: %3)").arg(id).arg(strength).arg(progStr));
    m_batchIdEdit->clear();
    
    saveToFile();
    populateCombos();
}

void MainWindow::onAddClassSession()
{
    QString instName = m_sessInstCombo->currentText();
    QString courseCode = m_sessCourseCombo->currentText();
    QString roomId = m_sessRoomCombo->currentText();
    QString batchId = m_sessBatchCombo->currentText();
    
    if (instName.isEmpty() || courseCode.isEmpty() || roomId.isEmpty() || batchId.isEmpty()) {
        QMessageBox::warning(this, "Selection Error", "Please ensure all entities (Instructors, Courses, Rooms, Batches) are created and selected.");
        return;
    }
    
    Instructor* inst = m_appManager.findInstructorByName(instName.toStdString());
    Course* crs = m_appManager.findCourseByCode(courseCode.toStdString());
    Room* rm = m_appManager.findRoomById(roomId.toStdString());
    StudentBatch* btch = m_appManager.findBatchById(batchId.toStdString());
    
    if (!inst || !crs || !rm || !btch) {
        QMessageBox::critical(this, "System Error", "Failed to retrieve references for the selected objects.");
        return;
    }
    
    QTime startTime = m_sessStartEdit->time();
    QTime endTime = m_sessEndEdit->time();
    
    if (startTime >= endTime) {
        QMessageBox::warning(this, "Time Range Error", "End time must be strictly after start time.");
        return;
    }
    
    Day day = static_cast<Day>(m_sessDayCombo->currentIndex());
    ClockTime ctStart{ startTime.hour(), startTime.minute() };
    ClockTime ctEnd{ endTime.hour(), endTime.minute() };
    TimeSlot slot(day, ctStart, ctEnd);
    
    // Assign course to instructor and track weekly hours workload limit
    if (!inst->assignNewCourse(*crs)) {
        int currentHours = inst->calculateTotalAssignedHours();
        QMessageBox::warning(this, "Workload Limit Exceeded",
            QString("Cannot schedule: %1 would exceed weekly hour limit!\n\n"
                    "Instructor: %2\n"
                    "Max Weekly Limit: %3 hours\n"
                    "Assigned so far: %4 hours\n"
                    "Course to assign: %5 (%6 hours)")
            .arg(QString::fromStdString(crs->getCourseCode()))
            .arg(QString::fromStdString(inst->getName()))
            .arg(inst->getMaxLimitHours())
            .arg(currentHours)
            .arg(QString::fromStdString(crs->getCourseCode()))
            .arg(crs->getAllocatedHours()));
        return;
    }
    
    // Create class session and add it
    ClassSession session(slot, inst, crs, rm, btch);
    m_appManager.addClassSession(session);
    
    saveToFile();
    refreshListsAndTables();
    
    QMessageBox::information(this, "Schedule Succeeded", "Class session scheduled successfully!");
}
