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
#include <QMenu>
#include <QAction>
#include <algorithm>

// ──────────────────────────────────────────────────────────────────────────────
// Static helpers
// ──────────────────────────────────────────────────────────────────────────────

// Convert Day enum → display string (handles all 7 days)
static QString dayToString(Day d) {
    switch (d) {
        case Day::Sunday:    return "Sunday";
        case Day::Monday:    return "Monday";
        case Day::Tuesday:   return "Tuesday";
        case Day::Wednesday: return "Wednesday";
        case Day::Thursday:  return "Thursday";
        case Day::Friday:    return "Friday";
        case Day::Saturday:  return "Saturday";
        default:             return "Unknown";
    }
}

static QString formatClockTime(ClockTime t) {
    return QString("%1:%2")
        .arg(t.hours,   2, 10, QChar('0'))
        .arg(t.minutes, 2, 10, QChar('0'));
}

// ──────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ──────────────────────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Catppuccin-inspired dark stylesheet (unchanged from original)
    this->setStyleSheet(R"(
        QMainWindow {
            background-color: #E8DDC7;
        }
        QTabWidget::pane {
            border: 1px solid #5A1810;
            background-color: #E8DDC7;
            border-radius: 8px;
        }
        QTabBar::tab {
            background: #E8DDC7;
            color: #6B5D48;
            border: 1px solid #5A1810;
            padding: 10px 20px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            font-weight: bold;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QTabBar::tab:selected, QTabBar::tab:hover {
            background: #E8DDC7;
            border-bottom-color: #E8DDC7;
            color: #A83A28;
        }
        QLabel {
            color: #2A1A12;
            font-size: 13px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
            font-weight: 500;
        }
        QLineEdit, QSpinBox, QComboBox, QTimeEdit {
            background-color: #D9C9A8;
            color: #2A1A12;
            border: 1px solid #5A1810;
            border-radius: 6px;
            padding: 6px;
            font-size: 13px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus, QTimeEdit:focus {
            border: 1px solid #A83A28;
        }
        QLineEdit:disabled, QSpinBox:disabled, QComboBox:disabled, QTimeEdit:disabled {
            background-color: #E8DDC7;
            color: #6B5D48;
            border: 1px solid #5A1810;
        }
        QPushButton {
            background-color: #A83A28;
            color: #FFFFFF;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
            font-size: 13px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QPushButton:hover {
            background-color: #6B1F16;
        }
        QPushButton:pressed {
            background-color: #6B1F16;
        }
        QPushButton:disabled {
            background-color: #D9C9A8;
            color: #6B5D48;
        }
        QListWidget, QTableWidget {
            background-color: #E8DDC7;
            color: #2A1A12;
            border: 1px solid #5A1810;
            border-radius: 8px;
            padding: 5px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QHeaderView::section {
            background-color: #D9C9A8;
            color: #2A1A12;
            border: 1px solid #5A1810;
            padding: 6px;
            font-weight: bold;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QTableWidget QTableCornerButton::section {
            background-color: #D9C9A8;
        }
        QCheckBox {
            color: #2A1A12;
            font-size: 13px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 2px solid #5A1810;
            border-radius: 4px;
            background-color: #D9C9A8;
        }
        QCheckBox::indicator:checked {
            background-color: #A83A28;
            border-color: #A83A28;
        }
        QCheckBox::indicator:disabled {
            background-color: #5A7A4A;
            border-color: #5A7A4A;
        }
        QGroupBox {
            color: #A83A28;
            font-size: 13px;
            font-weight: bold;
            font-family: 'Segoe UI', Helvetica, sans-serif;
            border: 1px solid #5A1810;
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 8px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 6px;
        }
        QTextEdit {
            background-color: #E8DDC7;
            color: #2A1A12;
            border: 1px solid #5A1810;
            border-radius: 8px;
            padding: 8px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
            font-size: 13px;
        }
        QScrollArea {
            background-color: transparent;
            border: none;
        }
        QScrollBar:vertical {
            background: #E8DDC7;
            width: 8px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical {
            background: #5A1810;
            border-radius: 4px;
            min-height: 20px;
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

// ──────────────────────────────────────────────────────────────────────────────
// setupUI
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::setupUI()
{
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);

    // =====================================================================
    // 1. INSTRUCTORS TAB
    // =====================================================================
    QWidget *instTab = new QWidget();
    QHBoxLayout *instLayout = new QHBoxLayout(instTab);

    QWidget *instLeft = new QWidget();
    QVBoxLayout *instFormLayout = new QVBoxLayout(instLeft);
    QFormLayout *instForm = new QFormLayout();

    m_instIdEdit    = new QLineEdit();
    m_instNameEdit  = new QLineEdit();
    m_instHoursSpin = new QSpinBox();
    m_instHoursSpin->setRange(1, 100);
    m_instHoursSpin->setValue(20);

    m_instSubjectCountSpin = new QSpinBox();
    m_instSubjectCountSpin->setRange(1, 3);
    m_instSubjectCountSpin->setValue(1);
    m_instSubjectCountSpin->setToolTip("How many subjects will this instructor teach?");

    instForm->addRow(new QLabel("Instructor ID:"),      m_instIdEdit);
    instForm->addRow(new QLabel("Instructor Name:"),    m_instNameEdit);
    instForm->addRow(new QLabel("Max Weekly Hours:"),   m_instHoursSpin);
    instForm->addRow(new QLabel("Number of Subjects:"), m_instSubjectCountSpin);

    m_instSubjectContainer = new QWidget();
    m_instSubjectLayout    = new QVBoxLayout(m_instSubjectContainer);
    m_instSubjectLayout->setContentsMargins(0, 0, 0, 0);
    m_instSubjectLayout->setSpacing(6);

    instFormLayout->addLayout(instForm);
    instFormLayout->addWidget(m_instSubjectContainer);

    connect(m_instSubjectCountSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onSubjectCountChanged);

    QPushButton *btnInstAdd    = new QPushButton("Add Instructor");
    QPushButton *btnInstEdit   = new QPushButton("Edit Instructor");
    QPushButton *btnInstDelete = new QPushButton("Delete Instructor");

    connect(btnInstAdd,    &QPushButton::clicked, this, &MainWindow::onAddInstructor);
    connect(btnInstEdit,   &QPushButton::clicked, this, &MainWindow::onEditInstructor);
    connect(btnInstDelete, &QPushButton::clicked, this, &MainWindow::onDeleteInstructor);

    QHBoxLayout *instBtnLayout = new QHBoxLayout();
    instBtnLayout->addWidget(btnInstAdd);
    instBtnLayout->addWidget(btnInstEdit);
    instBtnLayout->addWidget(btnInstDelete);

    instFormLayout->addLayout(instBtnLayout);
    instFormLayout->addStretch();

    m_instList = new QListWidget();
    instLayout->addWidget(instLeft, 1);
    instLayout->addWidget(m_instList, 2);
    m_tabWidget->addTab(instTab, "Instructors");

    rebuildSubjectCombos(1);

    // =====================================================================
    // 2. COURSES TAB
    // =====================================================================
    QWidget *courseTab = new QWidget();
    QHBoxLayout *courseLayout = new QHBoxLayout(courseTab);

    QWidget *courseLeft = new QWidget();
    QVBoxLayout *courseFormLayout = new QVBoxLayout(courseLeft);
    QFormLayout *courseForm = new QFormLayout();

    m_courseCodeEdit  = new QLineEdit();
    m_courseNameEdit  = new QLineEdit();
    m_courseHoursSpin = new QSpinBox();
    m_courseHoursSpin->setRange(1, 20);
    m_courseHoursSpin->setValue(3);

    courseForm->addRow(new QLabel("Course Code:"),  m_courseCodeEdit);
    courseForm->addRow(new QLabel("Course Name:"),  m_courseNameEdit);
    courseForm->addRow(new QLabel("Credit Hours:"), m_courseHoursSpin);

    QPushButton *btnCourseAdd    = new QPushButton("Add Course");
    QPushButton *btnCourseEdit   = new QPushButton("Edit Course");
    QPushButton *btnCourseDelete = new QPushButton("Delete Course");

    connect(btnCourseAdd,    &QPushButton::clicked, this, &MainWindow::onAddCourse);
    connect(btnCourseEdit,   &QPushButton::clicked, this, &MainWindow::onEditCourse);
    connect(btnCourseDelete, &QPushButton::clicked, this, &MainWindow::onDeleteCourse);

    QHBoxLayout *courseBtnLayout = new QHBoxLayout();
    courseBtnLayout->addWidget(btnCourseAdd);
    courseBtnLayout->addWidget(btnCourseEdit);
    courseBtnLayout->addWidget(btnCourseDelete);

    courseFormLayout->addLayout(courseForm);
    courseFormLayout->addLayout(courseBtnLayout);
    courseFormLayout->addStretch();

    m_courseList = new QListWidget();
    courseLayout->addWidget(courseLeft, 1);
    courseLayout->addWidget(m_courseList, 2);
    m_tabWidget->addTab(courseTab, "Courses");

    // =====================================================================
    // 3. ROOMS TAB
    // =====================================================================
    QWidget *roomTab = new QWidget();
    QHBoxLayout *roomLayout = new QHBoxLayout(roomTab);

    QWidget *roomLeft = new QWidget();
    QVBoxLayout *roomFormLayout = new QVBoxLayout(roomLeft);
    QFormLayout *roomForm = new QFormLayout();

    m_roomIdEdit       = new QLineEdit();
    m_roomBuildingEdit = new QLineEdit();
    m_roomCapSpin      = new QSpinBox();
    m_roomCapSpin->setRange(1, 500);
    m_roomCapSpin->setValue(60);
    m_roomTypeCombo = new QComboBox();
    m_roomTypeCombo->addItems({"Theory", "Lab", "Auditorium"});

    roomForm->addRow(new QLabel("Room Number:"), m_roomIdEdit);
    roomForm->addRow(new QLabel("Building:"),    m_roomBuildingEdit);
    roomForm->addRow(new QLabel("Capacity:"),    m_roomCapSpin);
    roomForm->addRow(new QLabel("Room Type:"),   m_roomTypeCombo);

    QPushButton *btnRoomAdd    = new QPushButton("Add Room");
    QPushButton *btnRoomEdit   = new QPushButton("Edit Room");
    QPushButton *btnRoomDelete = new QPushButton("Delete Room");

    connect(btnRoomAdd,    &QPushButton::clicked, this, &MainWindow::onAddRoom);
    connect(btnRoomEdit,   &QPushButton::clicked, this, &MainWindow::onEditRoom);
    connect(btnRoomDelete, &QPushButton::clicked, this, &MainWindow::onDeleteRoom);

    QHBoxLayout *roomBtnLayout = new QHBoxLayout();
    roomBtnLayout->addWidget(btnRoomAdd);
    roomBtnLayout->addWidget(btnRoomEdit);
    roomBtnLayout->addWidget(btnRoomDelete);

    roomFormLayout->addLayout(roomForm);
    roomFormLayout->addLayout(roomBtnLayout);
    roomFormLayout->addStretch();

    m_roomList = new QListWidget();
    roomLayout->addWidget(roomLeft, 1);
    roomLayout->addWidget(m_roomList, 2);
    m_tabWidget->addTab(roomTab, "Rooms");

    // =====================================================================
    // 4. STUDENT BATCHES TAB
    // =====================================================================
    QWidget *batchTab = new QWidget();
    QHBoxLayout *batchLayout = new QHBoxLayout(batchTab);

    QWidget *batchLeft = new QWidget();
    QVBoxLayout *batchFormLayout = new QVBoxLayout(batchLeft);
    QFormLayout *batchForm = new QFormLayout();

    m_batchIdEdit = new QLineEdit();
    m_batchStrengthSpin = new QSpinBox();
    m_batchStrengthSpin->setRange(1, 200);
    m_batchStrengthSpin->setValue(45);
    m_batchProgCombo = new QComboBox();
    m_batchProgCombo->addItems({"BIT", "BCE", "BCS"});
    m_batchDeptEdit = new QLineEdit();

    batchForm->addRow(new QLabel("Batch Name:"),  m_batchIdEdit);
    batchForm->addRow(new QLabel("Department:"),  m_batchDeptEdit);
    batchForm->addRow(new QLabel("Strength:"),    m_batchStrengthSpin);
    batchForm->addRow(new QLabel("Program:"),     m_batchProgCombo);

    QPushButton *btnBatchAdd    = new QPushButton("Add Batch");
    QPushButton *btnBatchEdit   = new QPushButton("Edit Batch");
    QPushButton *btnBatchDelete = new QPushButton("Delete Batch");

    connect(btnBatchAdd,    &QPushButton::clicked, this, &MainWindow::onAddBatch);
    connect(btnBatchEdit,   &QPushButton::clicked, this, &MainWindow::onEditBatch);
    connect(btnBatchDelete, &QPushButton::clicked, this, &MainWindow::onDeleteBatch);

    QHBoxLayout *batchBtnLayout = new QHBoxLayout();
    batchBtnLayout->addWidget(btnBatchAdd);
    batchBtnLayout->addWidget(btnBatchEdit);
    batchBtnLayout->addWidget(btnBatchDelete);

    batchFormLayout->addLayout(batchForm);
    batchFormLayout->addLayout(batchBtnLayout);
    batchFormLayout->addStretch();

    m_batchList = new QListWidget();
    batchLayout->addWidget(batchLeft, 1);
    batchLayout->addWidget(m_batchList, 2);
    m_tabWidget->addTab(batchTab, "Student Batches");

    // =====================================================================
    // 5. CONSTRAINTS TAB  (new)
    // =====================================================================
    setupConstraintsTab();

    // =====================================================================
    // 6. GENERATE & VIEW TIMETABLE TAB
    // =====================================================================
    QWidget *timetableTab = new QWidget();
    QHBoxLayout *timetableLayout = new QHBoxLayout(timetableTab);

    QWidget *timetableLeft = new QWidget();
    QVBoxLayout *timetableFormLayout = new QVBoxLayout(timetableLeft);

    // -- Dialog setup --
    m_addSessionDialog = new QDialog(this);
    m_addSessionDialog->setWindowTitle("Add Class Session");
    m_addSessionDialog->setModal(true);
    m_addSessionDialog->setStyleSheet("QDialog { background-color: #E8DDC7; color: #2A1A12; } QLabel { color: #2A1A12; }");
    QVBoxLayout *dialogLayout = new QVBoxLayout(m_addSessionDialog);

    QFormLayout *sessionForm = new QFormLayout();

    m_sessInstCombo   = new QComboBox();
    m_sessCourseCombo = new QComboBox();
    m_sessRoomCombo   = new QComboBox();
    m_sessBatchCombo  = new QComboBox();

    m_sessDayCombo = new QComboBox();
    m_sessDayCombo->addItems({"Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
                              "Sunday", "Saturday"});

    m_sessStartEdit = new QTimeEdit(QTime(9, 0));
    m_sessEndEdit   = new QTimeEdit(QTime(10, 0));
    m_sessStartEdit->setTimeRange(QTime(0, 0), QTime(23, 59));
    m_sessEndEdit->setTimeRange(QTime(0, 0), QTime(23, 59));

    sessionForm->addRow(new QLabel("Instructor:"),    m_sessInstCombo);
    sessionForm->addRow(new QLabel("Course:"),        m_sessCourseCombo);
    sessionForm->addRow(new QLabel("Room:"),          m_sessRoomCombo);
    sessionForm->addRow(new QLabel("Student Batch:"), m_sessBatchCombo);
    sessionForm->addRow(new QLabel("Day:"),           m_sessDayCombo);
    sessionForm->addRow(new QLabel("Start Time:"),    m_sessStartEdit);
    sessionForm->addRow(new QLabel("End Time:"),      m_sessEndEdit);

    dialogLayout->addLayout(sessionForm);

    QHBoxLayout *dialogBtnLayout = new QHBoxLayout();
    m_btnDialogSchedule = new QPushButton("Schedule Class Session");
    QPushButton *btnDialogCancel   = new QPushButton("Cancel");
    m_btnDialogDelete   = new QPushButton("Delete This Session");
    
    m_btnDialogSchedule->setStyleSheet(R"(
        QPushButton {
            background-color: #A83A28;
            color: #FFFFFF;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #6B1F16; }
    )");
    
    btnDialogCancel->setStyleSheet(R"(
        QPushButton {
            background-color: #D9C9A8;
            color: #FFFFFF;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
        }
        QPushButton:hover { background-color: #5A1810; }
    )");

    m_btnDialogDelete->setStyleSheet(R"(
        QPushButton {
            background-color: #C25B3A;
            color: #FFFFFF;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #C25B3A; }
    )");
    m_btnDialogDelete->setVisible(false); // Only shown in Edit mode

    dialogBtnLayout->addWidget(m_btnDialogDelete);
    dialogBtnLayout->addStretch();
    dialogBtnLayout->addWidget(btnDialogCancel);
    dialogBtnLayout->addWidget(m_btnDialogSchedule);
    dialogLayout->addLayout(dialogBtnLayout);

    connect(m_btnDialogSchedule, &QPushButton::clicked, this, &MainWindow::onAddClassSession);
    connect(btnDialogCancel,     &QPushButton::clicked, this, [this]() {
        resetSessionDialogToAddMode();
        m_addSessionDialog->reject();
    });
    connect(m_btnDialogDelete,   &QPushButton::clicked, this, [this]() {
        if (m_editingSessionId.isEmpty()) return;
        if (QMessageBox::question(this, "Confirm Delete",
                "Are you sure you want to remove this scheduled session?",
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
        if (m_appManager.removeClassSession(m_editingSessionId.toStdString())) {
            resetSessionDialogToAddMode();
            m_addSessionDialog->accept();
            saveToFile();
            refreshListsAndTables();
        } else {
            QMessageBox::warning(this, "Delete Failed",
                "Unable to remove the selected session.");
        }
    });


    // -- Main layout buttons --
    QPushButton *btnOpenAddDialog = new QPushButton("+ Add Class Session");
    btnOpenAddDialog->setStyleSheet(R"(
        QPushButton {
            background-color: #A83A28;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            padding: 12px 20px;
            font-weight: bold;
            font-size: 15px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QPushButton:hover { background-color: #6B1F16; }
    )");
    connect(btnOpenAddDialog, &QPushButton::clicked, this, [this]() {
        openSessionDialogForAdd();
    });


    QPushButton *btnSessionDelete = new QPushButton("Delete Class Session");
    btnSessionDelete->setStyleSheet(R"(
        QPushButton {
            background-color: #C25B3A;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            padding: 12px 20px;
            font-weight: bold;
            font-size: 15px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QPushButton:hover { background-color: #C25B3A; }
    )");
    connect(btnSessionDelete, &QPushButton::clicked, this, &MainWindow::onDeleteClassSession);

    // Auto Generate button — stored as member so we can enable/disable it
    m_btnAutoGenerate = new QPushButton("Auto Generate Routine");
    m_btnAutoGenerate->setEnabled(false);  // disabled until constraints are validated
    m_btnAutoGenerate->setStyleSheet(R"(
        QPushButton {
            background-color: #5A7A4A;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            padding: 12px 20px;
            font-weight: bold;
            font-size: 15px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QPushButton:hover { background-color: #5A7A4A; }
        QPushButton:disabled { background-color: #D9C9A8; color: #6B5D48; }
    )");
    connect(m_btnAutoGenerate, &QPushButton::clicked, this, &MainWindow::onAutoGenerate);

    // Status label under auto-generate button
    QLabel *genStatusLabel = new QLabel(
        "⚠  Validate constraints first (Constraints tab) before generating.");
    genStatusLabel->setWordWrap(true);
    genStatusLabel->setStyleSheet("color: #C25B3A; font-size: 12px;");
    genStatusLabel->setObjectName("genStatusLabel");

    timetableFormLayout->addWidget(btnOpenAddDialog);
    timetableFormLayout->addSpacing(10);
    timetableFormLayout->addWidget(m_btnAutoGenerate);
    timetableFormLayout->addWidget(genStatusLabel);
    timetableFormLayout->addSpacing(10);
    timetableFormLayout->addWidget(btnSessionDelete);
    timetableFormLayout->addStretch();

    m_timetableSubTabs = new QTabWidget();
    m_timetableSubTabs->setStyleSheet(R"(
        QTabWidget::pane { border: 1px solid #5A1810; background: #E8DDC7; }
        QTabBar::tab { background: #E8DDC7; color: #6B5D48; padding: 8px 16px; border: 1px solid #5A1810; }
        QTabBar::tab:selected { background: #E8DDC7; color: #A83A28; border-bottom-color: #E8DDC7; font-weight: bold; }
    )");

    // 1. "Schedule" Tab
    QWidget *flatTab = new QWidget();
    QVBoxLayout *flatLayout = new QVBoxLayout(flatTab);
    flatLayout->setContentsMargins(0, 0, 0, 0);
    m_timetableTable = new QTableWidget();
    m_timetableTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_timetableTable->setColumnCount(7);
    m_timetableTable->setHorizontalHeaderLabels({"Day", "Time", "Course", "Instructor", "Room", "Batch", "Duration"});
    flatLayout->addWidget(m_timetableTable);
    m_timetableSubTabs->addTab(flatTab, "Schedule");

    // 2. "Grid View" Tab
    QWidget *gridTab = new QWidget();
    gridTab->setObjectName("gridTabContainer");
    gridTab->setStyleSheet("QWidget#gridTabContainer { background-color: #E8DDC7; }");
    QVBoxLayout *gridLayout = new QVBoxLayout(gridTab);
    gridLayout->setContentsMargins(0, 8, 0, 0);

    m_viewBatchCombo = new QComboBox();
    m_viewBatchCombo->addItem("-- Select Batch to View --", QVariant(""));
    m_viewBatchCombo->setStyleSheet(R"(
        QComboBox { background-color: #D9C9A8; color: #A83A28; border: none; border-radius: 8px; padding: 4px 12px; font-weight: bold; }
        QComboBox::drop-down { border: none; }
        QComboBox:hover { background-color: #5A1810; }
    )");
    connect(m_viewBatchCombo, &QComboBox::currentIndexChanged, this, &MainWindow::onViewBatchChanged);

    m_btnRefreshGrid = new QPushButton("Refresh Grid");
    m_btnRefreshGrid->setStyleSheet(R"(
        QPushButton { background-color: #D9C9A8; color: #A83A28; border: none; border-radius: 8px; padding: 6px 12px; font-weight: bold; }
        QPushButton:hover { background-color: #5A1810; }
    )");
    connect(m_btnRefreshGrid, &QPushButton::clicked, this, &MainWindow::onRefreshGridClicked);

    QHBoxLayout *gridHeaderLayout = new QHBoxLayout();
    QLabel *lblDisplayBatch = new QLabel("Display Batch:");
    lblDisplayBatch->setStyleSheet("color: #A83A28; font-weight: bold;");
    gridHeaderLayout->addWidget(lblDisplayBatch);
    gridHeaderLayout->addWidget(m_viewBatchCombo);
    gridHeaderLayout->addWidget(m_btnRefreshGrid);
    gridHeaderLayout->addStretch();

    m_timetableGrid = new QTableWidget();
    m_timetableGrid->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_timetableGrid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_timetableGrid->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_timetableGrid->verticalHeader()->setMinimumSectionSize(115);
    m_timetableGrid->verticalHeader()->setMinimumWidth(80);
    m_timetableGrid->setStyleSheet(R"(
        QTableWidget {
            gridline-color: #E8DDC7;
            border: 2px solid #5A1810; 
            border-radius: 8px;
            background-color: #E8DDC7;
        }
        QHeaderView::section:horizontal {
            background-color: #8B2E1F;
            color: #E8DDC7;
            font-weight: 500;
            font-size: 11px;
            padding: 4px;
            border: none;
        }
        QHeaderView::section:vertical {
            background-color: #8B2E1F;
            color: #E8DDC7;
            font-weight: bold;
            font-size: 11px;
            padding: 4px;
            border: none;
        }
        QTableWidget::item {
            padding: 4px;
        }
    )");
    
    // Connect grid cell click for edit-in-place
    connect(m_timetableGrid, &QTableWidget::cellClicked,
            this, &MainWindow::onGridCellClicked);

    gridLayout->addLayout(gridHeaderLayout);
    gridLayout->addWidget(m_timetableGrid);
    m_timetableSubTabs->addTab(gridTab, "Grid View");


    QVBoxLayout *tableLayout = new QVBoxLayout();
    tableLayout->addWidget(m_timetableSubTabs);

    timetableLayout->addWidget(timetableLeft, 1);
    timetableLayout->addLayout(tableLayout, 2);
    m_tabWidget->addTab(timetableTab, "Generate & View Timetable");
}

// ──────────────────────────────────────────────────────────────────────────────
// setupConstraintsTab — builds the entire Constraints tab UI
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::setupConstraintsTab()
{
    QWidget *constraintsTab = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout(constraintsTab);

    // ── Left Panel (Scrollable) ────────────────────────────────────────────
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background-color: transparent; }");

    QWidget     *inner  = new QWidget();
    inner->setObjectName("constraintsInner");
    inner->setStyleSheet("#constraintsInner { background-color: transparent; }");
    QVBoxLayout *layout = new QVBoxLayout(inner);
    layout->setSpacing(16);
    layout->setContentsMargins(10, 10, 20, 10); // Some right margin before the splitter

    // ── Header ─────────────────────────────────────────────────────────────
    QLabel *headerLbl = new QLabel("⚙  Constraints & Rules");
    headerLbl->setStyleSheet("font-size: 18px; font-weight: bold;");
    layout->addWidget(headerLbl);

    QLabel *subLbl = new QLabel(
        "Configure scheduling constraints and run a feasibility check before generating the timetable.");
    subLbl->setWordWrap(true);
    layout->addWidget(subLbl);

    // ── Working Days ───────────────────────────────────────────────────────
    QGroupBox   *daysGroup  = new QGroupBox("Working Days  (unchecked = holiday, sessions never placed)");
    QGridLayout *daysLayout = new QGridLayout(daysGroup);
    daysLayout->setSpacing(8);

    const char* dayNames[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    // Defaults: Sun–Fri checked, Sat unchecked
    bool dayDefaults[7] = { true, true, true, true, true, true, false };

    const QString chipStyle = R"(
        QCheckBox {
            background-color: #E8DDC7;
            color: #2A1A12;
            border: 1px solid #5A1810;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QCheckBox::indicator {
            width: 0px; height: 0px;
        }
        QCheckBox:checked {
            background-color: #E8DDC7;
            color: #A83A28;
            border-color: #A83A28;
        }
        QCheckBox:hover {
            background-color: #E8DDC7;
        }
    )";

    for (int i = 0; i < 7; ++i) {
        m_dayChecks[i] = new QCheckBox(dayNames[i]);
        m_dayChecks[i]->setChecked(dayDefaults[i]);
        m_dayChecks[i]->setStyleSheet(chipStyle);
        connect(m_dayChecks[i], &QCheckBox::toggled, this, &MainWindow::onConstraintsChanged);
        daysLayout->addWidget(m_dayChecks[i], i / 4, i % 4);
    }
    daysLayout->setColumnStretch(4, 1); // Add a stretch column at the end to keep chips from expanding too much
    layout->addWidget(daysGroup);

    // ── Daily Time Window ──────────────────────────────────────────────────
    QGroupBox   *timeGroup  = new QGroupBox("Daily Time Window");
    QVBoxLayout *timeLayout = new QVBoxLayout(timeGroup);
    
    m_dayStartEdit = new QTimeEdit(QTime(9, 0));
    m_dayEndEdit   = new QTimeEdit(QTime(17, 0));
    m_dayStartEdit->setDisplayFormat("HH:mm");
    m_dayEndEdit->setDisplayFormat("HH:mm");
    connect(m_dayStartEdit, &QTimeEdit::timeChanged, this, &MainWindow::onConstraintsChanged);
    connect(m_dayEndEdit,   &QTimeEdit::timeChanged, this, &MainWindow::onConstraintsChanged);

    m_lunchEnabledCheck = new QCheckBox("Enable Lunch Break");
    m_lunchEnabledCheck->setChecked(true);
    connect(m_lunchEnabledCheck, &QCheckBox::toggled, this, &MainWindow::onConstraintsChanged);

    m_lunchStartEdit = new QTimeEdit(QTime(13, 0));
    m_lunchEndEdit   = new QTimeEdit(QTime(14, 0));
    m_lunchStartEdit->setDisplayFormat("HH:mm");
    m_lunchEndEdit->setDisplayFormat("HH:mm");
    connect(m_lunchStartEdit, &QTimeEdit::timeChanged, this, &MainWindow::onConstraintsChanged);
    connect(m_lunchEndEdit,   &QTimeEdit::timeChanged, this, &MainWindow::onConstraintsChanged);

    QHBoxLayout *timeRow1 = new QHBoxLayout();
    timeRow1->addWidget(new QLabel("Day Start:"));
    timeRow1->addWidget(m_dayStartEdit);
    timeRow1->addSpacing(15);
    timeRow1->addWidget(new QLabel("Day End:"));
    timeRow1->addWidget(m_dayEndEdit);
    timeRow1->addStretch();
    
    QHBoxLayout *timeRow2 = new QHBoxLayout();
    timeRow2->addWidget(m_lunchEnabledCheck);
    timeRow2->addSpacing(15);
    timeRow2->addWidget(new QLabel("Lunch Start:"));
    timeRow2->addWidget(m_lunchStartEdit);
    timeRow2->addSpacing(15);
    timeRow2->addWidget(new QLabel("Lunch End:"));
    timeRow2->addWidget(m_lunchEndEdit);
    timeRow2->addStretch();
    
    timeLayout->addLayout(timeRow1);
    timeLayout->addLayout(timeRow2);

    // Live capacity label
    m_capacityLabel = new QLabel("Available capacity: —");
    m_capacityLabel->setStyleSheet(
        "color: #A83A28; font-weight: bold; font-size: 13px; "
        "padding: 6px 10px; background-color: #E8DDC7; "
        "border: 1px solid #5A1810; border-radius: 6px;");
    timeLayout->addWidget(m_capacityLabel);

    layout->addWidget(timeGroup);

    // ── Scheduling Rules ───────────────────────────────────────────────────
    QGroupBox   *rulesGroup  = new QGroupBox("Scheduling Rules");
    QVBoxLayout *rulesLayout = new QVBoxLayout(rulesGroup);
    rulesLayout->setSpacing(10);

    auto makeRuleCheck = [&](QCheckBox*& chk, const QString& title, const QString& tooltip) {
        chk = new QCheckBox(title);
        chk->setToolTip(tooltip);
        chk->setChecked(true);
        connect(chk, &QCheckBox::toggled, this, &MainWindow::onConstraintsChanged);
    };

    makeRuleCheck(m_ruleNoInstDoubleBook, "No Instructor Clash", "Same instructor cannot be in two places at the same time");
    makeRuleCheck(m_ruleNoRoomDoubleBook, "No Room Clash", "Same room cannot hold two sessions at the same time");
    makeRuleCheck(m_ruleNoBatchClash, "No Batch Clash", "Same batch cannot have two sessions at the same time");
    makeRuleCheck(m_ruleInstDayGap, "Instructor Day Gap", "Instructor needs at least one gap day between teaching days");
    makeRuleCheck(m_ruleNoSameSubjectConsec, "Subject Day Gap", "A course can't repeat on back-to-back days for the same batch");
    makeRuleCheck(m_ruleMaxWeeklyHours, "Max Weekly Hours", "No instructor exceeds their configured weekly hour limit");
    makeRuleCheck(m_ruleMaxConsecHoursEnabled, "Max Daily Hours", "Caps consecutive teaching hours per instructor per day");

    // Max consecutive hours with spinner
    QHBoxLayout *consecRow = new QHBoxLayout();
    consecRow->setContentsMargins(0, 0, 0, 0);
    consecRow->addWidget(m_ruleMaxConsecHoursEnabled);
    m_maxConsecHoursSpin = new QSpinBox();
    m_maxConsecHoursSpin->setRange(1, 12);
    m_maxConsecHoursSpin->setValue(3);
    m_maxConsecHoursSpin->setFixedWidth(70);
    connect(m_maxConsecHoursSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onConstraintsChanged);
    consecRow->addWidget(m_maxConsecHoursSpin);
    consecRow->addStretch();

    // Enforce Instructor Subject Lock — always on, read-only
    m_ruleSubjectLock = new QCheckBox("Subject Lock (Locked)");
    m_ruleSubjectLock->setChecked(true);
    m_ruleSubjectLock->setEnabled(false);   // greyed-out / read-only
    m_ruleSubjectLock->setToolTip("Instructors only teach their assigned subject(s) — always on, core to the data model");

    // Add everything to the vertical layout
    rulesLayout->addWidget(m_ruleNoInstDoubleBook);
    rulesLayout->addWidget(m_ruleNoRoomDoubleBook);
    rulesLayout->addWidget(m_ruleNoBatchClash);
    rulesLayout->addWidget(m_ruleInstDayGap);
    rulesLayout->addWidget(m_ruleNoSameSubjectConsec);
    rulesLayout->addWidget(m_ruleMaxWeeklyHours);
    rulesLayout->addLayout(consecRow);
    rulesLayout->addWidget(m_ruleSubjectLock);

    layout->addWidget(rulesGroup);

    // ── Validate button ────────────────────────────────────────────────────
    QPushButton *btnValidate = new QPushButton("✔  Validate Constraints");
    connect(btnValidate, &QPushButton::clicked, this, &MainWindow::onValidateConstraints);
    layout->addWidget(btnValidate);

    layout->addStretch();
    scroll->setWidget(inner);
    
    mainLayout->addWidget(scroll, 1); // Give left panel stretch factor 1

    // ── Right Panel ────────────────────────────────────────────────────────
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(10, 10, 10, 10);

    QLabel *outLbl = new QLabel("Validation Results:");
    outLbl->setStyleSheet("font-size: 14px; font-weight: bold;");
    rightLayout->addWidget(outLbl);

    m_validationOutput = new QTextEdit();
    m_validationOutput->setReadOnly(true);
    m_validationOutput->setPlaceholderText(
        "Click \"Validate Constraints\" to run a feasibility check...");
    
    // Style text edit exactly like the list widgets in the other tabs
    m_validationOutput->setStyleSheet(
        "QTextEdit {"
        "  background-color: #E8DDC7;"
        "  color: #2A1A12;"
        "  border: 1px solid #5A1810;"
        "  border-radius: 8px;"
        "  padding: 5px;"
        "  font-family: 'Segoe UI', Helvetica, sans-serif;"
        "}"
    );

    rightLayout->addWidget(m_validationOutput);
    
    mainLayout->addWidget(rightPanel, 2); // Give right panel stretch factor 2

    m_tabWidget->addTab(constraintsTab, "Constraints");

    // Compute initial capacity label
    updateCapacityLabel();
}

// ──────────────────────────────────────────────────────────────────────────────
// Constraint helpers
// ──────────────────────────────────────────────────────────────────────────────

ConstraintSettings MainWindow::readConstraintsFromUI() const
{
    ConstraintSettings cs;

    // Working days  [0]=Sun,[1]=Mon,...,[6]=Sat
    for (int i = 0; i < 7; ++i)
        cs.workingDays[i] = m_dayChecks[i]->isChecked();

    QTime start = m_dayStartEdit->time();
    QTime end   = m_dayEndEdit->time();
    cs.dayStartMinutes = start.hour() * 60 + start.minute();
    cs.dayEndMinutes   = end.hour()   * 60 + end.minute();

    cs.lunchBreakEnabled = m_lunchEnabledCheck->isChecked();
    QTime ls = m_lunchStartEdit->time();
    QTime le = m_lunchEndEdit->time();
    cs.lunchStartMinutes = ls.hour() * 60 + ls.minute();
    cs.lunchEndMinutes   = le.hour() * 60 + le.minute();

    cs.ruleNoInstructorDoubleBook  = m_ruleNoInstDoubleBook->isChecked();
    cs.ruleNoRoomDoubleBook        = m_ruleNoRoomDoubleBook->isChecked();
    cs.ruleNoBatchClash            = m_ruleNoBatchClash->isChecked();
    cs.ruleInstructorDayGap        = m_ruleInstDayGap->isChecked();
    cs.ruleNoSameSubjectConsecDays = m_ruleNoSameSubjectConsec->isChecked();
    cs.ruleRespectMaxWeeklyHours   = m_ruleMaxWeeklyHours->isChecked();
    cs.ruleMaxConsecHoursEnabled   = m_ruleMaxConsecHoursEnabled->isChecked();
    cs.ruleMaxConsecHoursPerDay    = m_maxConsecHoursSpin->value();
    cs.ruleEnforceSubjectLock      = true;  // always on

    return cs;
}

void MainWindow::updateCapacityLabel()
{
    ConstraintSettings cs = readConstraintsFromUI();

    int workingDayCount = 0;
    for (int i = 0; i < 7; ++i)
        if (cs.workingDays[i]) ++workingDayCount;

    int dailyMinutes = cs.dayEndMinutes - cs.dayStartMinutes;
    if (cs.lunchBreakEnabled) {
        int lunchLen = cs.lunchEndMinutes - cs.lunchStartMinutes;
        if (lunchLen > 0) dailyMinutes -= lunchLen;
    }
    if (dailyMinutes < 0) dailyMinutes = 0;

    int totalWeeklyMins = workingDayCount * dailyMinutes;
    double weeklyHours  = totalWeeklyMins / 60.0;

    m_capacityLabel->setText(
        QString("Available capacity:  %1 hrs/week  (%2 working day%3 × %4 hrs/day)")
            .arg(weeklyHours, 0, 'f', 1)
            .arg(workingDayCount)
            .arg(workingDayCount != 1 ? "s" : "")
            .arg(dailyMinutes / 60.0, 0, 'f', 1));
}

void MainWindow::markConstraintsDirty()
{
    m_constraintsValidated = false;
    if (m_btnAutoGenerate) {
        m_btnAutoGenerate->setEnabled(false);
        m_btnAutoGenerate->setToolTip(
            "Constraints have not been validated. "
            "Go to the Constraints tab and click Validate.");
    }
}

void MainWindow::onConstraintsChanged()
{
    markConstraintsDirty();
    updateCapacityLabel();
}

// ──────────────────────────────────────────────────────────────────────────────
// onValidateConstraints — feasibility pre-check
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::onValidateConstraints()
{
    ConstraintSettings cs = readConstraintsFromUI();
    m_constraints = cs;

    QString output;
    bool allPassed = true;

    auto pass = [&](const QString& msg) {
        output += "<span style='color:#5A7A4A'>&#10003; " + msg + "</span><br>";
    };
    auto fail = [&](const QString& msg) {
        output += "<span style='color:#C25B3A'>&#10007; " + msg + "</span><br>";
        allPassed = false;
    };
    auto info = [&](const QString& msg) {
        output += "<span style='color:#6B5D48'>&#9432; " + msg + "</span><br>";
    };

    output += "<b style='color:#A83A28'>─── Structural Checks ───</b><br>";

    // 1. At least one working day
    int workingDayCount = 0;
    for (int i = 0; i < 7; ++i) if (cs.workingDays[i]) ++workingDayCount;

    if (workingDayCount > 0)
        pass(QString("Working days selected: %1").arg(workingDayCount));
    else
        fail("No working days selected — cannot schedule any sessions.");

    // 2. Time window validity
    if (cs.dayStartMinutes < cs.dayEndMinutes)
        pass(QString("Day window valid: %1:%2 – %3:%4")
             .arg(cs.dayStartMinutes/60,2,10,QChar('0'))
             .arg(cs.dayStartMinutes%60,2,10,QChar('0'))
             .arg(cs.dayEndMinutes/60,2,10,QChar('0'))
             .arg(cs.dayEndMinutes%60,2,10,QChar('0')));
    else
        fail("Day Start must be earlier than Day End.");

    // 3. Lunch break validity (if enabled)
    if (cs.lunchBreakEnabled) {
        if (cs.lunchStartMinutes >= cs.dayStartMinutes &&
            cs.lunchEndMinutes   <= cs.dayEndMinutes   &&
            cs.lunchStartMinutes <  cs.lunchEndMinutes)
            pass(QString("Lunch break valid: %1:%2 – %3:%4")
                 .arg(cs.lunchStartMinutes/60,2,10,QChar('0'))
                 .arg(cs.lunchStartMinutes%60,2,10,QChar('0'))
                 .arg(cs.lunchEndMinutes/60,2,10,QChar('0'))
                 .arg(cs.lunchEndMinutes%60,2,10,QChar('0')));
        else
            fail("Lunch break window is invalid or lies outside the day window.");
    } else {
        info("Lunch break disabled.");
    }

    // 4. Data availability
    output += "<br><b style='color:#A83A28'>─── Data Availability ───</b><br>";

    const auto& courses     = m_appManager.getCourses();
    const auto& instructors = m_appManager.getInstructors();
    const auto& rooms       = m_appManager.getRooms();
    const auto& batches     = m_appManager.getBatches();

    if (courses.empty())     fail("No courses defined.");
    if (instructors.empty()) fail("No instructors defined.");
    if (rooms.empty())       fail("No rooms defined.");
    if (batches.empty())     fail("No student batches defined.");

    if (!courses.empty() && !instructors.empty() && !rooms.empty() && !batches.empty())
        pass(QString("Data loaded: %1 course(s), %2 instructor(s), %3 room(s), %4 batch(es).")
             .arg(courses.size()).arg(instructors.size()).arg(rooms.size()).arg(batches.size()));

    // 5. Weekly capacity check
    output += "<br><b style='color:#A83A28'>─── Weekly Capacity (per batch) ───</b><br>";

    int dailyMinutes = cs.dayEndMinutes - cs.dayStartMinutes;
    if (cs.lunchBreakEnabled) {
        int ll = cs.lunchEndMinutes - cs.lunchStartMinutes;
        if (ll > 0) dailyMinutes -= ll;
    }
    if (dailyMinutes < 0) dailyMinutes = 0;
    int weeklyCapMins  = workingDayCount * dailyMinutes;
    int weeklyCapHours = weeklyCapMins / 60;

    // Total course hours needed by each batch
    int totalCourseHours = 0;
    for (const auto& crs : courses)
        totalCourseHours += crs.getAllocatedHours();

    for (const auto& b : batches) {
        if (totalCourseHours <= weeklyCapHours) {
            pass(QString("Weekly capacity sufficient for %1: needs %2 hrs, available %3 hrs/week.")
                 .arg(QString::fromStdString(b.getBatchId()))
                 .arg(totalCourseHours)
                 .arg(weeklyCapHours));
        } else {
            fail(QString("Weekly capacity INSUFFICIENT for %1: needs %2 hrs but only %3 hrs/week available.")
                 .arg(QString::fromStdString(b.getBatchId()))
                 .arg(totalCourseHours)
                 .arg(weeklyCapHours));
        }
    }

    // 6. Each course has at least one qualified instructor
    output += "<br><b style='color:#A83A28'>─── Instructor Qualification ───</b><br>";

    for (const auto& crs : courses) {
        bool found = false;
        for (const auto& inst : instructors) {
            if (inst.isQualifiedFor(crs.getCourseCode())) { found = true; break; }
        }
        if (found)
            pass(QString("Course \"%1\" has a qualified instructor.")
                 .arg(QString::fromStdString(crs.getCourseCode())));
        else
            fail(QString("Course \"%1\" has NO qualified instructor assigned!")
                 .arg(QString::fromStdString(crs.getCourseCode())));
    }

    // 7. Instructor workload vs their max hours
    output += "<br><b style='color:#A83A28'>─── Instructor Workload ───</b><br>";

    for (const auto& crs : courses) {
        int courseHoursNeeded = crs.getAllocatedHours() * static_cast<int>(batches.size());
        int totalCapacity = 0;
        QStringList qualifiedInstNames;
        
        for (const auto& inst : instructors) {
            if (inst.isQualifiedFor(crs.getCourseCode())) {
                totalCapacity += inst.getMaxLimitHours();
                qualifiedInstNames << QString("%1: %2 hrs")
                                      .arg(QString::fromStdString(inst.getName()))
                                      .arg(inst.getMaxLimitHours());
            }
        }
        
        if (totalCapacity == 0) {
            // Already handled by 'Instructor Qualification' check, but good to catch here too.
        } else if (courseHoursNeeded > totalCapacity) {
            fail(QString("Course %1 needs %2 hrs/week total across all batches, but qualified instructors (%3) only provide %4 hrs combined capacity — insufficient.")
                 .arg(QString::fromStdString(crs.getCourseCode()))
                 .arg(courseHoursNeeded)
                 .arg(qualifiedInstNames.join(", "))
                 .arg(totalCapacity));
        } else {
            pass(QString("Course %1 capacity OK (%2 hrs needed ≤ %3 hrs combined capacity).")
                 .arg(QString::fromStdString(crs.getCourseCode()))
                 .arg(courseHoursNeeded)
                 .arg(totalCapacity));
        }
    }

    // 8. Room count adequacy
    output += "<br><b style='color:#A83A28'>─── Room Availability ───</b><br>";

    int roomCount  = static_cast<int>(rooms.size());
    int batchCount = static_cast<int>(batches.size());
    if (roomCount >= batchCount)
        pass(QString("Room count (%1) is sufficient for the number of simultaneous batches (%2).")
             .arg(roomCount).arg(batchCount));
    else
        fail(QString("Only %1 room(s) but %2 concurrent batch(es) may need scheduling simultaneously.")
             .arg(roomCount).arg(batchCount));

    // ── Final verdict ───────────────────────────────────────────────────────
    output += "<br>";
    if (allPassed) {
        output += "<b style='color:#5A7A4A; font-size:14px'>"
                  "✔  All checks passed — you may now Auto Generate the timetable.</b>";
        m_constraintsValidated = true;
        m_constraints = cs;
        m_btnAutoGenerate->setEnabled(true);
        m_btnAutoGenerate->setToolTip("Constraints validated — ready to generate.");
    } else {
        output += "<b style='color:#C25B3A; font-size:14px'>"
                  "✘  Some checks failed — please fix the issues above before generating.</b>";
        m_constraintsValidated = false;
        m_btnAutoGenerate->setEnabled(false);
    }

    m_validationOutput->setHtml(output);
}

// ──────────────────────────────────────────────────────────────────────────────
// Subject combo helpers
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::rebuildSubjectCombos(int count)
{
    // Clear out the previous layout completely before adding new ones
    QLayoutItem *child;
    while ((child = m_instSubjectLayout->takeAt(0)) != nullptr) {
        if (QWidget *widget = child->widget()) {
            widget->deleteLater();
        }
        delete child;
    }
    m_instSubjectCombos.clear();

    for (int i = 0; i < count; ++i) {
        QLabel *lbl = new QLabel(QString("Subject %1:").arg(i + 1));
        lbl->setFixedWidth(90);

        QComboBox *cb = new QComboBox();
        cb->addItem("-- Select Course --", QVariant(""));
        for (const auto& crs : m_appManager.getCourses()) {
            cb->addItem(QString::fromStdString(crs.getCourseCode()), 
                        QVariant(QString::fromStdString(crs.getCourseCode())));
        }

        QWidget *rowWidget = new QWidget();
        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        rowLayout->addWidget(lbl);
        rowLayout->addWidget(cb);

        m_instSubjectLayout->addWidget(rowWidget);
        m_instSubjectCombos.append(cb);
    }
}

void MainWindow::onSubjectCountChanged(int count)
{
    rebuildSubjectCombos(count);
}

// ──────────────────────────────────────────────────────────────────────────────
// Instructor list helpers
// ──────────────────────────────────────────────────────────────────────────────

static QString instDisplayString(const Instructor& inst)
{
    QString base = QString("%1 (Max Hours: %2)")
        .arg(QString::fromStdString(inst.getName()))
        .arg(inst.getMaxLimitHours());

    const auto& locked = inst.getLockedSubjects();
    if (!locked.empty()) {
        QStringList subjects;
        for (const auto& s : locked)
            subjects << QString::fromStdString(s);
        base += QString(" \u2014 Subjects: %1 [LOCKED]").arg(subjects.join(", "));
    }
    return base;
}

void MainWindow::refreshInstList()
{
    m_instList->clear();
    for (const auto& inst : m_appManager.getInstructors())
        m_instList->addItem(instDisplayString(inst));
}

// ──────────────────────────────────────────────────────────────────────────────
// populateInitialData
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::populateInitialData()
{
    m_appManager.addCourse(Course("COMP-102", "Computer Programming", 3));
    m_appManager.addCourse(Course("MATH-101", "Mathematics I", 4));

    {
        Instructor inst("Dr. Niraj Sharma", 12);
        inst.setLockedSubjects({"COMP-102"});
        m_appManager.addInstructor(inst);
    }
    {
        Instructor inst("Prof. Ram Prasad", 15);
        inst.setLockedSubjects({"MATH-101"});
        m_appManager.addInstructor(inst);
    }

    m_appManager.addRoom(Room("Block-C-102", 60, RoomType::Theory));
    m_appManager.addRoom(Room("Lab-A-301",   40, RoomType::Lab));

    m_appManager.addBatch(StudentBatch("BCT-2025-A", 48, ProgramType::BCE));
    m_appManager.addBatch(StudentBatch("BIT-2025-B", 45, ProgramType::BIT));

    refreshInstList();
    m_courseList->addItem("COMP-102 (Allocated Hours: 3)");
    m_courseList->addItem("MATH-101 (Allocated Hours: 4)");
    m_roomList->addItem("Block-C-102 (Capacity: 60, Type: Theory)");
    m_roomList->addItem("Lab-A-301 (Capacity: 40, Type: Lab)");
    m_batchList->addItem("BCT-2025-A (Strength: 48, Program: BCE)");
    m_batchList->addItem("BIT-2025-B (Strength: 45, Program: BIT)");
}

// ──────────────────────────────────────────────────────────────────────────────
// saveToFile
// ──────────────────────────────────────────────────────────────────────────────

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
        instObj["id"]            = QString::fromStdString(inst.getId());
        instObj["name"]          = QString::fromStdString(inst.getName());
        instObj["maxLimitHours"] = inst.getMaxLimitHours();

        QJsonArray lockedArray;
        for (const auto& s : inst.getLockedSubjects())
            lockedArray.append(QString::fromStdString(s));
        instObj["lockedSubjects"] = lockedArray;

        QJsonArray assignedArray;
        instObj["assignedCourses"] = assignedArray;

        instArray.append(instObj);
    }
    rootObj["instructors"] = instArray;

    // 2. Courses
    QJsonArray courseArray;
    for (const auto& crs : m_appManager.getCourses()) {
        QJsonObject crsObj;
        crsObj["code"]           = QString::fromStdString(crs.getCode());
        crsObj["name"]           = QString::fromStdString(crs.getName());
        crsObj["creditHours"]    = crs.getCreditHours();
        crsObj["courseCode"]     = QString::fromStdString(crs.getCourseCode());
        crsObj["allocatedHours"] = crs.getAllocatedHours();
        courseArray.append(crsObj);
    }
    rootObj["courses"] = courseArray;

    // 3. Rooms
    QJsonArray roomArray;
    for (const auto& rm : m_appManager.getRooms()) {
        QJsonObject rmObj;
        rmObj["roomId"]   = QString::fromStdString(rm.getRoomId());
        rmObj["capacity"] = rm.getCapacity();
        rmObj["type"]     = static_cast<int>(rm.getType());
        rmObj["building"] = QString::fromStdString(rm.getBuilding());
        roomArray.append(rmObj);
    }
    rootObj["rooms"] = roomArray;

    // 4. Batches
    QJsonArray batchArray;
    for (const auto& b : m_appManager.getBatches()) {
        QJsonObject bObj;
        bObj["batchId"]    = QString::fromStdString(b.getBatchId());
        bObj["strength"]   = b.getStrength();
        bObj["program"]    = static_cast<int>(b.getProgram());
        bObj["department"] = QString::fromStdString(b.getDepartment());
        batchArray.append(bObj);
    }
    rootObj["batches"] = batchArray;

    // 5. Timetable
    QJsonArray sessionArray;
    for (const auto& s : m_appManager.getTimetable()) {
        QJsonObject sObj;
        TimeSlot ts = s.getTimeSlot();
        sObj["day"]        = static_cast<int>(ts.getDay());
        sObj["startH"]     = ts.getStartTime().hours;
        sObj["startM"]     = ts.getStartTime().minutes;
        sObj["endH"]       = ts.getEndTime().hours;
        sObj["endM"]       = ts.getEndTime().minutes;
        sObj["instructor"] = QString::fromStdString(s.getTeacherId()->getId());
        sObj["course"]     = QString::fromStdString(s.getSubjectId()->getCourseCode());
        sObj["room"]       = QString::fromStdString(s.getRoomId()->getRoomId());
        sObj["batch"]      = QString::fromStdString(s.getBatchId()->getBatchId());
        sObj["sessionId"]  = QString::fromStdString(s.getSessionId());
        sessionArray.append(sObj);
    }
    rootObj["timetable"] = sessionArray;

    // 6. Constraints settings
    QJsonObject csObj;
    ConstraintSettings cs = readConstraintsFromUI();
    QJsonArray daysArray;
    for (int i = 0; i < 7; ++i) daysArray.append(cs.workingDays[i]);
    csObj["workingDays"]           = daysArray;
    csObj["dayStartMinutes"]       = cs.dayStartMinutes;
    csObj["dayEndMinutes"]         = cs.dayEndMinutes;
    csObj["lunchBreakEnabled"]     = cs.lunchBreakEnabled;
    csObj["lunchStartMinutes"]     = cs.lunchStartMinutes;
    csObj["lunchEndMinutes"]       = cs.lunchEndMinutes;
    csObj["ruleNoInstDoubleBook"]  = cs.ruleNoInstructorDoubleBook;
    csObj["ruleNoRoomDoubleBook"]  = cs.ruleNoRoomDoubleBook;
    csObj["ruleNoBatchClash"]      = cs.ruleNoBatchClash;
    csObj["ruleInstDayGap"]        = cs.ruleInstructorDayGap;
    csObj["ruleNoSameSubjConsec"]  = cs.ruleNoSameSubjectConsecDays;
    csObj["ruleMaxWeeklyHours"]    = cs.ruleRespectMaxWeeklyHours;
    csObj["ruleMaxConsecEnabled"]  = cs.ruleMaxConsecHoursEnabled;
    csObj["ruleMaxConsecHours"]    = cs.ruleMaxConsecHoursPerDay;
    rootObj["constraints"] = csObj;

    QJsonDocument doc(rootObj);
    file.write(doc.toJson());
    file.close();
}

// ──────────────────────────────────────────────────────────────────────────────
// loadFromFile
// ──────────────────────────────────────────────────────────────────────────────

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

    // 1. Courses
    if (rootObj.contains("courses") && rootObj["courses"].isArray()) {
        for (const auto& val : rootObj["courses"].toArray()) {
            QJsonObject crsObj = val.toObject();
            std::string code = crsObj.contains("code")
                               ? crsObj["code"].toString().toStdString()
                               : crsObj["courseCode"].toString().toStdString();
            std::string name = crsObj.contains("name")
                               ? crsObj["name"].toString().toStdString() : code;
            int creditHours  = crsObj.contains("creditHours")
                               ? crsObj["creditHours"].toInt()
                               : crsObj["allocatedHours"].toInt();
            m_appManager.addCourse(Course(code, name, creditHours));
        }
    }

    // 2. Instructors
    if (rootObj.contains("instructors") && rootObj["instructors"].isArray()) {
        for (const auto& val : rootObj["instructors"].toArray()) {
            QJsonObject instObj = val.toObject();
            std::string id   = instObj.contains("id")
                               ? instObj["id"].toString().toStdString()
                               : instObj["name"].toString().toStdString();
            std::string name = instObj["name"].toString().toStdString();
            int maxHours     = instObj["maxLimitHours"].toInt();

            Instructor inst(id, name, maxHours);

            if (instObj.contains("lockedSubjects") && instObj["lockedSubjects"].isArray()) {
                std::vector<std::string> locked;
                for (const auto& lv : instObj["lockedSubjects"].toArray())
                    locked.push_back(lv.toString().toStdString());
                inst.setLockedSubjects(locked);
            }
            if (instObj.contains("assignedCourses") && instObj["assignedCourses"].isArray()) {
                // Legacy field ignored — assigned hours are derived directly from the timetable at runtime.
            }
            m_appManager.addInstructor(inst);
        }
    }

    // 3. Rooms
    if (rootObj.contains("rooms") && rootObj["rooms"].isArray()) {
        for (const auto& val : rootObj["rooms"].toArray()) {
            QJsonObject rmObj = val.toObject();
            std::string id    = rmObj["roomId"].toString().toStdString();
            int cap           = rmObj["capacity"].toInt();
            RoomType type     = static_cast<RoomType>(rmObj["type"].toInt());
            std::string building = rmObj.contains("building")
                                   ? rmObj["building"].toString().toStdString() : "Main";
            m_appManager.addRoom(Room(id, cap, type, building));
        }
    }

    // 4. Batches
    if (rootObj.contains("batches") && rootObj["batches"].isArray()) {
        for (const auto& val : rootObj["batches"].toArray()) {
            QJsonObject bObj = val.toObject();
            std::string id   = bObj["batchId"].toString().toStdString();
            int strength     = bObj["strength"].toInt();
            ProgramType prog = static_cast<ProgramType>(bObj["program"].toInt());
            std::string dept = bObj.contains("department")
                               ? bObj["department"].toString().toStdString() : "CS";
            m_appManager.addBatch(StudentBatch(id, strength, prog, dept));
        }
    }

    // 5. Timetable
    if (rootObj.contains("timetable") && rootObj["timetable"].isArray()) {
        for (const auto& val : rootObj["timetable"].toArray()) {
            QJsonObject sObj = val.toObject();
            Day day = static_cast<Day>(sObj["day"].toInt());
            ClockTime ctStart{ sObj["startH"].toInt(), sObj["startM"].toInt() };
            ClockTime ctEnd{   sObj["endH"].toInt(),   sObj["endM"].toInt()   };
            TimeSlot slot(day, ctStart, ctEnd);

            Instructor*   inst = m_appManager.findInstructorById(
                sObj["instructor"].toString().toStdString());
            if (!inst) inst = m_appManager.findInstructorByName(
                sObj["instructor"].toString().toStdString());
            Course*       crs  = m_appManager.findCourseByCode(
                sObj["course"].toString().toStdString());
            Room*         rm   = m_appManager.findRoomById(
                sObj["room"].toString().toStdString());
            StudentBatch* btch = m_appManager.findBatchById(
                sObj["batch"].toString().toStdString());
            std::string sessionId = sObj.contains("sessionId") ? sObj["sessionId"].toString().toStdString() : "";

            if (inst && crs && rm && btch) {
                // Use default CS (no lunch enforcement on load — just restore sessions)
                ConstraintSettings loadCS;
                loadCS.lunchBreakEnabled = false;
                m_appManager.validateAndAddClassSession(
                    ClassSession(slot, inst, crs, rm, btch, sessionId), loadCS);
            }
        }
    }

    // 6. Constraints (optional section — present only in new save files)
    if (rootObj.contains("constraints") && rootObj["constraints"].isObject()) {
        QJsonObject csObj = rootObj["constraints"].toObject();

        if (csObj.contains("workingDays") && csObj["workingDays"].isArray()) {
            QJsonArray arr = csObj["workingDays"].toArray();
            for (int i = 0; i < 7 && i < arr.size(); ++i)
                m_dayChecks[i]->setChecked(arr[i].toBool());
        }

        auto loadTime = [&](const QString& key, QTimeEdit* te) {
            if (csObj.contains(key)) {
                int mins = csObj[key].toInt();
                te->setTime(QTime(mins / 60, mins % 60));
            }
        };
        loadTime("dayStartMinutes",   m_dayStartEdit);
        loadTime("dayEndMinutes",     m_dayEndEdit);
        loadTime("lunchStartMinutes", m_lunchStartEdit);
        loadTime("lunchEndMinutes",   m_lunchEndEdit);

        if (csObj.contains("lunchBreakEnabled"))
            m_lunchEnabledCheck->setChecked(csObj["lunchBreakEnabled"].toBool());

        auto loadRule = [&](const QString& key, QCheckBox* chk) {
            if (csObj.contains(key)) chk->setChecked(csObj[key].toBool());
        };
        loadRule("ruleNoInstDoubleBook", m_ruleNoInstDoubleBook);
        loadRule("ruleNoRoomDoubleBook", m_ruleNoRoomDoubleBook);
        loadRule("ruleNoBatchClash",     m_ruleNoBatchClash);
        loadRule("ruleInstDayGap",       m_ruleInstDayGap);
        loadRule("ruleNoSameSubjConsec", m_ruleNoSameSubjectConsec);
        loadRule("ruleMaxWeeklyHours",   m_ruleMaxWeeklyHours);
        loadRule("ruleMaxConsecEnabled", m_ruleMaxConsecHoursEnabled);

        if (csObj.contains("ruleMaxConsecHours"))
            m_maxConsecHoursSpin->setValue(csObj["ruleMaxConsecHours"].toInt());
    }

    // Rebuild list widgets
    refreshInstList();
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

    rebuildSubjectCombos(m_instSubjectCountSpin->value());
    updateCapacityLabel();
}

// ──────────────────────────────────────────────────────────────────────────────
// populateCombos
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::populateCombos()
{
    m_sessInstCombo->clear();
    m_sessCourseCombo->clear();
    m_sessRoomCombo->clear();
    m_sessBatchCombo->clear();

    for (const auto& inst : m_appManager.getInstructors())
        m_sessInstCombo->addItem(QString::fromStdString(inst.getName()));

    m_viewBatchCombo->clear();
    m_viewBatchCombo->addItem("-- Select Batch to View --", QVariant(""));
    
    for (const auto& crs : m_appManager.getCourses()) {
        m_sessCourseCombo->addItem(QString::fromStdString(crs.getCourseCode()), 
                                   QVariant(QString::fromStdString(crs.getCourseCode())));
    }
    for (const auto& room : m_appManager.getRooms()) {
        m_sessRoomCombo->addItem(QString::fromStdString(room.getRoomId()), 
                                 QVariant(QString::fromStdString(room.getRoomId())));
    }
    for (const auto& batch : m_appManager.getBatches()) {
        m_sessBatchCombo->addItem(QString::fromStdString(batch.getBatchId()), 
                                  QVariant(QString::fromStdString(batch.getBatchId())));
        m_viewBatchCombo->addItem(QString::fromStdString(batch.getBatchId()), 
                                  QVariant(QString::fromStdString(batch.getBatchId())));
    }

    rebuildSubjectCombos(m_instSubjectCountSpin->value());
}

// ──────────────────────────────────────────────────────────────────────────────
// refreshListsAndTables
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::onViewBatchChanged()
{
    refreshTimetableGrid();
}

static QMap<QString, QColor> buildCourseColorMap(const std::vector<Course>& courses) {
    QMap<QString, QColor> colorMap;
    const std::vector<std::string> palette = {
        "#DDB1BB", // saturated dusty rose
        "#BDCDA9", // sage/olive green
        "#E8B9AA", // warm terracotta
        "#9DCBCA", // medium teal-green
        "#A8C6EA", // solid medium blue
        "#D7ADC1", // rich mauve
        "#C8D8B6", // rich sage
        "#E8C1AB", // rich beige/copper
        "#ADC8D8", // rich dusty blue
        "#E1B1C7"  // rich muted pink
    };
    int index = 0;
    for (const auto& crs : courses) {
        QString code = QString::fromStdString(crs.getCourseCode());
        if (!colorMap.contains(code)) {
            if (index >= static_cast<int>(palette.size())) {
                qWarning("Warning: Ran out of unique colors in palette, cycling colors for %s", qPrintable(code));
            }
            colorMap[code] = QColor(QString::fromStdString(palette[index % palette.size()]));
            index++;
        }
    }
    return colorMap;
}

void MainWindow::refreshListsAndTables()
{
    const auto& timetable = m_appManager.getTimetable();
    m_timetableTable->setRowCount(0);
    for (const auto& session : timetable) {
        int row = m_timetableTable->rowCount();
        m_timetableTable->insertRow(row);

        TimeSlot ts = session.getTimeSlot();
        QString timeStr = formatClockTime(ts.getStartTime()) + " - " + formatClockTime(ts.getEndTime());

        QTableWidgetItem* dayItem = new QTableWidgetItem(dayToString(ts.getDay()));
        dayItem->setData(Qt::UserRole, QString::fromStdString(session.getSessionId()));
        m_timetableTable->setItem(row, 0, dayItem);
        m_timetableTable->setItem(row, 1, new QTableWidgetItem(timeStr));
        m_timetableTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(session.getSubjectId()->getCourseCode())));
        m_timetableTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(session.getTeacherId()->getName())));
        m_timetableTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(session.getRoomId()->getRoomId())));
        m_timetableTable->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(session.getBatchId()->getBatchId())));
        m_timetableTable->setItem(row, 6, new QTableWidgetItem(QString("%1 mins").arg(ts.getDurationmin())));
    }

    refreshTimetableGrid();
}

void MainWindow::onRefreshGridClicked()
{
    refreshTimetableGrid();
}

// ──────────────────────────────────────────────────────────────────────────────
// Edit-in-place helpers
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::onGridCellClicked(int row, int col)
{
    QTableWidgetItem *item = m_timetableGrid->item(row, col);
    if (!item) return;

    QVariant sid = item->data(Qt::UserRole);
    if (sid.isNull() || sid.toString().isEmpty()) {
        // Empty cell — open Add dialog pre-filled with day/time from this cell
        openSessionDialogForAdd(row, col);
    } else {
        QString sidStr = sid.toString();
        if (sidStr.contains(",")) {
            // It's a merged block. Show a menu.
            QMenu menu(this);
            QStringList sids = sidStr.split(",");
            for (const QString& id : sids) {
                const ClassSession* target = nullptr;
                for (const auto& sess : m_appManager.getTimetable()) {
                    if (sess.getSessionId() == id.toStdString()) {
                        target = &sess;
                        break;
                    }
                }
                if (target) {
                    int h1 = target->getTimeSlot().getStartTime().hours;
                    int m1 = target->getTimeSlot().getStartTime().minutes;
                    int h2 = (h1 * 60 + m1 + target->getTimeSlot().getDurationmin()) / 60;
                    int m2 = (h1 * 60 + m1 + target->getTimeSlot().getDurationmin()) % 60;
                    QString timeStr = QString("%1:%2 - %3:%4")
                        .arg(h1, 2, 10, QChar('0'))
                        .arg(m1, 2, 10, QChar('0'))
                        .arg(h2, 2, 10, QChar('0'))
                        .arg(m2, 2, 10, QChar('0'));
                    
                    QAction* action = menu.addAction("Edit Session: " + timeStr);
                    connect(action, &QAction::triggered, this, [this, id]() {
                        openSessionDialogForEdit(id.toStdString());
                    });
                }
            }
            menu.exec(QCursor::pos());
        } else {
            // Occupied cell — open Edit dialog pre-filled with this session's data
            openSessionDialogForEdit(sidStr.toStdString());
        }
    }
}

void MainWindow::resetSessionDialogToAddMode()
{
    m_editingSessionId.clear();
    m_addSessionDialog->setWindowTitle("Add Class Session");
    m_btnDialogSchedule->setText("Schedule Class Session");
    m_btnDialogDelete->setVisible(false);
}

void MainWindow::openSessionDialogForEdit(const std::string &sessionId)
{
    // Locate the session
    const ClassSession *target = nullptr;
    for (const auto &sess : m_appManager.getTimetable()) {
        if (sess.getSessionId() == sessionId) {
            target = &sess;
            break;
        }
    }
    if (!target) {
        QMessageBox::warning(this, "Not Found",
            "The selected session could not be found. Please refresh the grid.");
        return;
    }

    // Refresh combo contents before pre-filling
    populateCombos();

    // Pre-fill Instructor
    QString instName = QString::fromStdString(target->getTeacherId()->getName());
    int instIdx = m_sessInstCombo->findText(instName);
    if (instIdx != -1) m_sessInstCombo->setCurrentIndex(instIdx);

    // Pre-fill Course
    QString courseCode = QString::fromStdString(target->getSubjectId()->getCourseCode());
    int crsIdx = m_sessCourseCombo->findData(QVariant(courseCode));
    if (crsIdx != -1) m_sessCourseCombo->setCurrentIndex(crsIdx);

    // Pre-fill Room
    QString roomId = QString::fromStdString(target->getRoomId()->getRoomId());
    int rmIdx = m_sessRoomCombo->findData(QVariant(roomId));
    if (rmIdx != -1) m_sessRoomCombo->setCurrentIndex(rmIdx);

    // Pre-fill Batch
    QString batchId = QString::fromStdString(target->getBatchId()->getBatchId());
    int batchIdx = m_sessBatchCombo->findData(QVariant(batchId));
    if (batchIdx != -1) m_sessBatchCombo->setCurrentIndex(batchIdx);

    // Pre-fill Day — map Day enum to combo index
    // Combo order: Monday(0), Tuesday(1), Wednesday(2), Thursday(3), Friday(4), Sunday(5), Saturday(6)
    const Day dayMap[] = {
        Day::Monday, Day::Tuesday, Day::Wednesday, Day::Thursday,
        Day::Friday, Day::Sunday, Day::Saturday
    };
    Day sessionDay = target->getTimeSlot().getDay();
    for (int i = 0; i < 7; ++i) {
        if (dayMap[i] == sessionDay) {
            m_sessDayCombo->setCurrentIndex(i);
            break;
        }
    }

    // Pre-fill Start / End times
    ClockTime st = target->getTimeSlot().getStartTime();
    ClockTime et = target->getTimeSlot().getEndTime();
    m_sessStartEdit->setTime(QTime(st.hours, st.minutes));
    m_sessEndEdit->setTime(QTime(et.hours, et.minutes));

    // Enter Edit mode
    m_editingSessionId = QString::fromStdString(sessionId);
    m_addSessionDialog->setWindowTitle("Edit Class Session");
    m_btnDialogSchedule->setText("Save Changes");
    m_btnDialogDelete->setVisible(true);

    m_addSessionDialog->exec();

    // Always reset to Add mode after dialog closes (accept or reject)
    resetSessionDialogToAddMode();
}

void MainWindow::openSessionDialogForAdd(int prefillDayRow, int prefillColSlot)
{
    // Ensure we are in clean Add mode
    resetSessionDialogToAddMode();
    populateCombos();

    if (prefillDayRow >= 0 && prefillColSlot >= 0) {
        // Pre-fill Day from vertical header
        QTableWidgetItem *vHeader = m_timetableGrid->verticalHeaderItem(prefillDayRow);
        if (vHeader) {
            QString dayText = vHeader->text();
            // dayText is e.g. "Monday", "Tuesday" etc. — find matching combo entry
            for (int i = 0; i < m_sessDayCombo->count(); ++i) {
                if (m_sessDayCombo->itemText(i) == dayText) {
                    m_sessDayCombo->setCurrentIndex(i);
                    break;
                }
            }
        }

        // Pre-fill Start Time from horizontal header (format "HH:mm - HH:mm")
        QTableWidgetItem *hHeader = m_timetableGrid->horizontalHeaderItem(prefillColSlot);
        if (hHeader) {
            QString headerText = hHeader->text().split("\n").first(); // strip "(Lunch)" if present
            QStringList parts  = headerText.split(" - ");
            if (parts.size() >= 2) {
                QTime startT = QTime::fromString(parts[0].trimmed(), "HH:mm");
                QTime endT   = QTime::fromString(parts[1].trimmed(), "HH:mm");
                if (startT.isValid()) m_sessStartEdit->setTime(startT);
                if (endT.isValid())   m_sessEndEdit->setTime(endT);
            }
        }
    }

    m_addSessionDialog->exec();

    // Ensure clean state after dialog closes
    resetSessionDialogToAddMode();
}

void MainWindow::refreshTimetableGrid()
{
    m_timetableGrid->clear();
    m_timetableGrid->clearSpans();

    QString batchId = m_viewBatchCombo->currentData().toString();
    if (batchId.isEmpty()) {
        m_timetableGrid->setRowCount(0);
        m_timetableGrid->setColumnCount(0);
        return;
    }

    QMap<QString, QColor> courseColors = buildCourseColorMap(m_appManager.getCourses());

    ConstraintSettings cs = readConstraintsFromUI();
    std::vector<Day> workDays;
    for (int i = 0; i < 7; ++i) {
        if (cs.workingDays[i]) workDays.push_back(static_cast<Day>(i));
    }

    m_timetableGrid->setRowCount(workDays.size());
    QStringList vHeaders;
    for (Day d : workDays) vHeaders << dayToString(d);
    m_timetableGrid->setVerticalHeaderLabels(vHeaders);

    int totalMinutes = cs.dayEndMinutes - cs.dayStartMinutes;
    int cols = totalMinutes / 60;
    if (cols < 0) cols = 0;
    m_timetableGrid->setColumnCount(cols);

    QStringList hHeaders;
    for (int i = 0; i < cols; ++i) {
        int startMin = cs.dayStartMinutes + i * 60;
        int endMin = startMin + 60;
        ClockTime startCT = {startMin / 60, startMin % 60};
        ClockTime endCT = {endMin / 60, endMin % 60};
        
        QString headerText = formatClockTime(startCT) + " - " + formatClockTime(endCT);
        if (cs.lunchBreakEnabled && startMin >= cs.lunchStartMinutes && startMin < cs.lunchEndMinutes) {
            headerText += "\n(Lunch)";
        }
        hHeaders << headerText;
    }
    m_timetableGrid->setHorizontalHeaderLabels(hHeaders);

    // Initialize grid with blanks and lunch breaks
    for (size_t r = 0; r < workDays.size(); ++r) {
        for (int c = 0; c < cols; ++c) {
            int startMin = cs.dayStartMinutes + c * 60;
            bool isLunch = false;
            if (cs.lunchBreakEnabled && startMin >= cs.lunchStartMinutes && startMin < cs.lunchEndMinutes) {
                isLunch = true;
            }
            
            QTableWidgetItem *item = new QTableWidgetItem("-X-");
            item->setTextAlignment(Qt::AlignCenter);
            QFont font = item->font();
            font.setPointSize(11);
            item->setFont(font);
            if (isLunch) {
                item->setBackground(QColor("#D9C9A8"));
                item->setForeground(QColor("#6B5D48"));
            } else {
                item->setBackground(QColor("#E8DDC7"));
                item->setForeground(QColor("#5A1810"));
            }
            m_timetableGrid->setItem(r, c, item);
        }
    }

    // Populate actual sessions
    std::vector<ClassSession> batchSessions;
    for (const auto& session : m_appManager.getTimetable()) {
        if (QString::fromStdString(session.getBatchId()->getBatchId()) == batchId) {
            batchSessions.push_back(session);
        }
    }

    std::sort(batchSessions.begin(), batchSessions.end(), [](const ClassSession& a, const ClassSession& b) {
        if (a.getTimeSlot().getDay() != b.getTimeSlot().getDay()) {
            return static_cast<int>(a.getTimeSlot().getDay()) < static_cast<int>(b.getTimeSlot().getDay());
        }
        int startA = a.getTimeSlot().getStartTime().hours * 60 + a.getTimeSlot().getStartTime().minutes;
        int startB = b.getTimeSlot().getStartTime().hours * 60 + b.getTimeSlot().getStartTime().minutes;
        return startA < startB;
    });

    std::vector<std::vector<ClassSession>> mergedGroups;
    if (!batchSessions.empty()) {
        std::vector<ClassSession> currentGroup;
        currentGroup.push_back(batchSessions[0]);
        
        for (size_t i = 1; i < batchSessions.size(); ++i) {
            const ClassSession& prev = currentGroup.back();
            const ClassSession& curr = batchSessions[i];
            
            bool canMerge = true;
            if (prev.getTimeSlot().getDay() != curr.getTimeSlot().getDay()) canMerge = false;
            
            if (canMerge) {
                if (prev.getSubjectId()->getCourseCode() != curr.getSubjectId()->getCourseCode() ||
                    prev.getTeacherId()->getId() != curr.getTeacherId()->getId() ||
                    prev.getRoomId()->getRoomId() != curr.getRoomId()->getRoomId()) {
                    canMerge = false;
                }
            }
            
            if (canMerge) {
                int prevEnd = prev.getTimeSlot().getStartTime().hours * 60 + prev.getTimeSlot().getStartTime().minutes + prev.getTimeSlot().getDurationmin();
                int currStart = curr.getTimeSlot().getStartTime().hours * 60 + curr.getTimeSlot().getStartTime().minutes;
                if (prevEnd != currStart) canMerge = false;
                
                if (cs.lunchBreakEnabled) {
                    if (prevEnd > cs.lunchStartMinutes && currStart < cs.lunchEndMinutes) {
                        canMerge = false;
                    } else if (prevEnd == cs.lunchStartMinutes && currStart == cs.lunchEndMinutes) {
                        canMerge = false;
                    } else if (prevEnd <= cs.lunchStartMinutes && currStart >= cs.lunchEndMinutes) {
                        canMerge = false;
                    }
                }
            }
            
            if (canMerge) {
                currentGroup.push_back(curr);
            } else {
                mergedGroups.push_back(currentGroup);
                currentGroup.clear();
                currentGroup.push_back(curr);
            }
        }
        mergedGroups.push_back(currentGroup);
    }

    for (const auto& group : mergedGroups) {
        if (group.empty()) continue;
        const ClassSession& firstSession = group.front();
        TimeSlot ts = firstSession.getTimeSlot();
        
        int r = -1;
        for (size_t i = 0; i < workDays.size(); ++i) {
            if (workDays[i] == ts.getDay()) { r = i; break; }
        }
        if (r == -1) continue;

        int startMin = ts.getStartTime().hours * 60 + ts.getStartTime().minutes;
        int startSlot = (startMin - cs.dayStartMinutes) / 60;
        
        int totalSpanSlots = 0;
        QStringList sessionIds;
        for (const auto& sess : group) {
            totalSpanSlots += sess.getTimeSlot().getDurationmin() / 60;
            sessionIds << QString::fromStdString(sess.getSessionId());
        }

        if (startSlot < 0 || startSlot + totalSpanSlots > cols) continue;

        QString fullInstName = QString::fromStdString(firstSession.getTeacherId()->getName());

        QString plainText = QString("%1\n%2\n%3")
            .arg(QString::fromStdString(firstSession.getSubjectId()->getCourseCode()))
            .arg(fullInstName)
            .arg(QString::fromStdString(firstSession.getRoomId()->getRoomId()));

        QString cellHtml = QString(
            "<div style='text-align: center; color: #2A1A12; line-height: 1.15;'>"
            "<div style='font-size: 11pt; font-weight: bold;'>%1</div>"
            "<div style='font-size: 9.5pt; font-weight: 500; margin-top: 2px; margin-bottom: 2px;'>%2</div>"
            "<div style='font-size: 8.5pt; font-weight: 300; color: #6B5D48;'>%3</div>"
            "</div>"
        )
            .arg(QString::fromStdString(firstSession.getSubjectId()->getCourseCode()))
            .arg(fullInstName)
            .arg(QString::fromStdString(firstSession.getRoomId()->getRoomId()));

        QTableWidgetItem *item = new QTableWidgetItem();
        item->setToolTip(plainText);
        
        QString cCode = QString::fromStdString(firstSession.getSubjectId()->getCourseCode());
        QColor bg = courseColors.value(cCode, QColor("#ADC8D8")); // default fallback
        item->setBackground(bg);
        item->setData(Qt::UserRole, QVariant(sessionIds.join(",")));

        m_timetableGrid->setItem(r, startSlot, item);
        
        QLabel *lbl = new QLabel(cellHtml);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setWordWrap(true);
        lbl->setAttribute(Qt::WA_TransparentForMouseEvents);
        lbl->setStyleSheet("background: transparent;");
        m_timetableGrid->setCellWidget(r, startSlot, lbl);

        if (totalSpanSlots > 1) {
            m_timetableGrid->setSpan(r, startSlot, 1, totalSpanSlots);
        }
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// INSTRUCTOR CRUD
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::onAddInstructor()
{
    std::string id   = m_instIdEdit->text().trimmed().toStdString();
    std::string name = m_instNameEdit->text().trimmed().toStdString();
    int maxHours     = m_instHoursSpin->value();

    if (id.empty()) {
        QMessageBox::warning(this, "Validation Error", "Instructor ID cannot be empty.");
        return;
    }
    if (name.empty()) {
        QMessageBox::warning(this, "Validation Error", "Instructor Name cannot be empty.");
        return;
    }

    bool isEditing = !m_editingInstId.empty();

    if (!isEditing && m_appManager.findInstructorById(id)) {
        QMessageBox::warning(this, "Validation Error", "Instructor ID must be unique.");
        return;
    }

    std::vector<std::string> lockedSubjects;
    if (isEditing && m_appManager.isInstructorUsed(m_editingInstId)) {
        Instructor* existing = m_appManager.findInstructorById(m_editingInstId);
        if (existing) lockedSubjects = existing->getLockedSubjects();
    } else {
        for (int i = 0; i < m_instSubjectCombos.size(); ++i) {
            QComboBox* cb = m_instSubjectCombos[i];
            if (cb->currentIndex() == 0) {
                QMessageBox::warning(this, "Validation Error",
                    QString("Please select a course for Subject %1.").arg(i + 1));
                return;
            }
            std::string code = cb->currentData().toString().toStdString();
            for (const auto& already : lockedSubjects) {
                if (already == code) {
                    QMessageBox::warning(this, "Validation Error",
                        QString("Subject %1 (\"%2\") is already selected.")
                            .arg(i + 1).arg(cb->currentText()));
                    return;
                }
            }
            lockedSubjects.push_back(code);
        }
        if (lockedSubjects.empty()) {
            QMessageBox::warning(this, "Validation Error",
                "At least one subject must be assigned to the instructor.");
            return;
        }
    }

    Instructor inst(id, name, maxHours);
    inst.setLockedSubjects(lockedSubjects);

    if (isEditing) {

        m_appManager.updateInstructor(inst);
        m_editingInstId = "";
        m_instIdEdit->setEnabled(true);
        m_instSubjectCountSpin->setEnabled(true);
    } else {
        m_appManager.addInstructor(inst);
    }

    m_instIdEdit->clear();
    m_instNameEdit->clear();
    m_instHoursSpin->setValue(20);
    m_instSubjectCountSpin->setValue(1);
    m_instSubjectCountSpin->setEnabled(true);
    rebuildSubjectCombos(1);

    refreshInstList();
    saveToFile();
    populateCombos();
    markConstraintsDirty();
}

void MainWindow::onEditInstructor()
{
    QListWidgetItem *item = m_instList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Selection Error", "No instructor selected to edit.");
        return;
    }

    QString text = item->text();
    int parenIdx = text.indexOf(" (Max Hours:");
    QString displayName = (parenIdx != -1) ? text.left(parenIdx) : text;

    Instructor* inst = m_appManager.findInstructorByName(displayName.toStdString());
    if (!inst) inst  = m_appManager.findInstructorById(displayName.toStdString());
    if (!inst) {
        QMessageBox::warning(this, "Error", "Selected instructor not found.");
        return;
    }

    m_instIdEdit->setText(QString::fromStdString(inst->getId()));
    m_instNameEdit->setText(QString::fromStdString(inst->getName()));
    m_instHoursSpin->setValue(inst->getMaxLimitHours());

    m_editingInstId = inst->getId();
    m_instIdEdit->setEnabled(false);

    const auto& locked = inst->getLockedSubjects();
    bool isUsed = m_appManager.isInstructorUsed(inst->getId());

    if (isUsed) {
        m_instSubjectCountSpin->setValue(static_cast<int>(locked.size() > 0 ? locked.size() : 1));
        m_instSubjectCountSpin->setEnabled(false);
        rebuildSubjectCombos(static_cast<int>(locked.size() > 0 ? locked.size() : 1));
        for (int i = 0; i < m_instSubjectCombos.size() && i < static_cast<int>(locked.size()); ++i) {
            int idx = m_instSubjectCombos[i]->findData(QString::fromStdString(locked[i]));
            if (idx != -1) m_instSubjectCombos[i]->setCurrentIndex(idx);
            m_instSubjectCombos[i]->setEnabled(false);
        }
        QMessageBox::information(this, "Subject List Locked",
            "This instructor has scheduled sessions. Name and Max Hours can still be edited,\n"
            "but the subject list is locked and cannot be changed.");
    } else {
        int subCount = static_cast<int>(locked.size() > 0 ? locked.size() : 1);
        m_instSubjectCountSpin->setValue(subCount);
        m_instSubjectCountSpin->setEnabled(true);
        rebuildSubjectCombos(subCount);
        for (int i = 0; i < m_instSubjectCombos.size() && i < static_cast<int>(locked.size()); ++i) {
            int idx = m_instSubjectCombos[i]->findData(QString::fromStdString(locked[i]));
            if (idx != -1) m_instSubjectCombos[i]->setCurrentIndex(idx);
        }
    }
}

void MainWindow::onDeleteInstructor()
{
    QListWidgetItem *item = m_instList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Selection Error", "No instructor selected to delete.");
        return;
    }

    QString text = item->text();
    int parenIdx = text.indexOf(" (Max Hours:");
    QString displayName = (parenIdx != -1) ? text.left(parenIdx) : text;

    Instructor* inst = m_appManager.findInstructorByName(displayName.toStdString());
    if (!inst) inst  = m_appManager.findInstructorById(displayName.toStdString());
    if (!inst) {
        QMessageBox::warning(this, "Error", "Selected instructor not found.");
        return;
    }

    if (m_appManager.isInstructorUsed(inst->getId())) {
        QMessageBox::critical(this, "Cannot Delete",
            "This instructor is used in scheduled sessions and cannot be deleted.");
        return;
    }

    if (QMessageBox::question(this, "Confirm Delete",
            "Are you sure you want to delete this instructor?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    if (m_appManager.removeInstructor(inst->getId())) {
        delete item;
        saveToFile();
        populateCombos();
        markConstraintsDirty();
    } else {
        QMessageBox::warning(this, "Delete Failed", "Unable to delete instructor.");
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// COURSE CRUD
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::onAddCourse()
{
    std::string code = m_courseCodeEdit->text().trimmed().toStdString();
    std::string name = m_courseNameEdit->text().trimmed().toStdString();
    int hours        = m_courseHoursSpin->value();

    if (code.empty()) {
        QMessageBox::warning(this, "Validation Error", "Course Code cannot be empty.");
        return;
    }
    if (m_editingCourseCode.empty() || m_editingCourseCode != code) {
        if (m_appManager.findCourseByCode(code)) {
            QMessageBox::warning(this, "Validation Error", "Course Code must be unique.");
            return;
        }
    }
    if (name.empty()) {
        QMessageBox::warning(this, "Validation Error", "Course Name cannot be empty.");
        return;
    }

    Course crs(code, name, hours);

    if (!m_editingCourseCode.empty()) {
        m_appManager.updateCourse(crs);
        m_editingCourseCode = "";
        m_courseCodeEdit->setEnabled(true);
    } else {
        m_appManager.addCourse(crs);
    }

    m_courseCodeEdit->clear();
    m_courseNameEdit->clear();
    m_courseHoursSpin->setValue(3);

    m_courseList->clear();
    for (const auto& c : m_appManager.getCourses()) {
        m_courseList->addItem(QString("%1 (Allocated Hours: %2)")
            .arg(QString::fromStdString(c.getCourseCode()))
            .arg(c.getAllocatedHours()));
    }

    saveToFile();
    populateCombos();
    markConstraintsDirty();
}

void MainWindow::onEditCourse()
{
    QListWidgetItem *item = m_courseList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Selection Error", "No course selected to edit.");
        return;
    }

    QString text = item->text();
    int idx = text.indexOf(" (Allocated Hours:");
    std::string code = (idx != -1) ? text.left(idx).toStdString() : text.toStdString();

    Course* crs = m_appManager.findCourseByCode(code);
    if (!crs) {
        QMessageBox::warning(this, "Error", "Selected course not found.");
        return;
    }

    m_courseCodeEdit->setText(QString::fromStdString(crs->getCourseCode()));
    m_courseNameEdit->setText(QString::fromStdString(crs->getName()));
    m_courseHoursSpin->setValue(crs->getAllocatedHours());

    m_editingCourseCode = crs->getCourseCode();
    m_courseCodeEdit->setEnabled(false);
}

void MainWindow::onDeleteCourse()
{
    QListWidgetItem *item = m_courseList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Selection Error", "No course selected to delete.");
        return;
    }

    QString text = item->text();
    int idx = text.indexOf(" (Allocated Hours:");
    std::string code = (idx != -1) ? text.left(idx).toStdString() : text.toStdString();

    if (m_appManager.isCourseUsed(code)) {
        QMessageBox::critical(this, "Cannot Delete",
            "This course is used in scheduled sessions and cannot be deleted.");
        return;
    }

    if (QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this course?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    if (m_appManager.removeCourse(code)) {
        delete item;
        saveToFile();
        populateCombos();
        markConstraintsDirty();
    } else {
        QMessageBox::warning(this, "Delete Failed", "Unable to delete course.");
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// ROOM CRUD
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::onAddRoom()
{
    std::string number   = m_roomIdEdit->text().trimmed().toStdString();
    std::string building = m_roomBuildingEdit->text().trimmed().toStdString();
    int capacity         = m_roomCapSpin->value();
    RoomType type        = static_cast<RoomType>(m_roomTypeCombo->currentIndex());

    if (number.empty()) {
        QMessageBox::warning(this, "Validation Error", "Room Number cannot be empty.");
        return;
    }
    if (m_editingRoomId.empty() || m_editingRoomId != number) {
        if (m_appManager.findRoomById(number)) {
            QMessageBox::warning(this, "Validation Error", "Room Number must be unique.");
            return;
        }
    }
    if (building.empty()) {
        QMessageBox::warning(this, "Validation Error", "Building cannot be empty.");
        return;
    }

    Room rm(number, capacity, type, building);

    if (!m_editingRoomId.empty()) {
        m_appManager.updateRoom(rm);
        m_editingRoomId = "";
        m_roomIdEdit->setEnabled(true);
    } else {
        m_appManager.addRoom(rm);
    }

    m_roomIdEdit->clear();
    m_roomBuildingEdit->clear();
    m_roomCapSpin->setValue(60);
    m_roomTypeCombo->setCurrentIndex(0);

    m_roomList->clear();
    for (const auto& r : m_appManager.getRooms()) {
        m_roomList->addItem(QString("%1 (Capacity: %2, Type: %3)")
            .arg(QString::fromStdString(r.getRoomId()))
            .arg(r.getCapacity())
            .arg(QString::fromStdString(r.getTypeAsString())));
    }

    saveToFile();
    populateCombos();
    markConstraintsDirty();
}

void MainWindow::onEditRoom()
{
    QListWidgetItem *item = m_roomList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Selection Error", "No room selected to edit.");
        return;
    }

    QString text = item->text();
    int idx = text.indexOf(" (Capacity:");
    std::string number = (idx != -1) ? text.left(idx).toStdString() : text.toStdString();

    Room* rm = m_appManager.findRoomById(number);
    if (!rm) {
        QMessageBox::warning(this, "Error", "Selected room not found.");
        return;
    }

    m_roomIdEdit->setText(QString::fromStdString(rm->getRoomId()));
    m_roomBuildingEdit->setText(QString::fromStdString(rm->getBuilding()));
    m_roomCapSpin->setValue(rm->getCapacity());
    m_roomTypeCombo->setCurrentIndex(static_cast<int>(rm->getType()));

    m_editingRoomId = rm->getRoomId();
    m_roomIdEdit->setEnabled(false);
}

void MainWindow::onDeleteRoom()
{
    QListWidgetItem *item = m_roomList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Selection Error", "No room selected to delete.");
        return;
    }

    QString text = item->text();
    int idx = text.indexOf(" (Capacity:");
    std::string number = (idx != -1) ? text.left(idx).toStdString() : text.toStdString();

    if (m_appManager.isRoomUsed(number)) {
        QMessageBox::critical(this, "Cannot Delete",
            "This room is used in scheduled sessions and cannot be deleted.");
        return;
    }

    if (QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this room?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    if (m_appManager.removeRoom(number)) {
        delete item;
        saveToFile();
        populateCombos();
        markConstraintsDirty();
    } else {
        QMessageBox::warning(this, "Delete Failed", "Unable to delete room.");
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// BATCH CRUD
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::onAddBatch()
{
    std::string name = m_batchIdEdit->text().trimmed().toStdString();
    std::string dept = m_batchDeptEdit->text().trimmed().toStdString();
    int strength     = m_batchStrengthSpin->value();
    ProgramType prog = static_cast<ProgramType>(m_batchProgCombo->currentIndex());

    if (name.empty()) {
        QMessageBox::warning(this, "Validation Error", "Batch Name cannot be empty.");
        return;
    }
    if (m_editingBatchId.empty() || m_editingBatchId != name) {
        if (m_appManager.findBatchById(name)) {
            QMessageBox::warning(this, "Validation Error", "Batch Name must be unique.");
            return;
        }
    }
    if (dept.empty()) {
        QMessageBox::warning(this, "Validation Error", "Department cannot be empty.");
        return;
    }

    StudentBatch b(name, strength, prog, dept);

    if (!m_editingBatchId.empty()) {
        m_appManager.updateBatch(b);
        m_editingBatchId = "";
        m_batchIdEdit->setEnabled(true);
    } else {
        m_appManager.addBatch(b);
    }

    m_batchIdEdit->clear();
    m_batchDeptEdit->clear();
    m_batchStrengthSpin->setValue(45);
    m_batchProgCombo->setCurrentIndex(0);

    m_batchList->clear();
    for (const auto& bat : m_appManager.getBatches()) {
        m_batchList->addItem(QString("%1 (Strength: %2, Program: %3)")
            .arg(QString::fromStdString(bat.getBatchId()))
            .arg(bat.getStrength())
            .arg(QString::fromStdString(bat.getProgramAsString())));
    }

    saveToFile();
    populateCombos();
    markConstraintsDirty();
}

void MainWindow::onEditBatch()
{
    QListWidgetItem *item = m_batchList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Selection Error", "No student batch selected to edit.");
        return;
    }

    QString text = item->text();
    int idx = text.indexOf(" (Strength:");
    std::string name = (idx != -1) ? text.left(idx).toStdString() : text.toStdString();

    StudentBatch* b = m_appManager.findBatchById(name);
    if (!b) {
        QMessageBox::warning(this, "Error", "Selected student batch not found.");
        return;
    }

    m_batchIdEdit->setText(QString::fromStdString(b->getBatchId()));
    m_batchDeptEdit->setText(QString::fromStdString(b->getDepartment()));
    m_batchStrengthSpin->setValue(b->getStrength());
    m_batchProgCombo->setCurrentIndex(static_cast<int>(b->getProgram()));

    m_editingBatchId = b->getBatchId();
    m_batchIdEdit->setEnabled(false);
}

void MainWindow::onDeleteBatch()
{
    QListWidgetItem *item = m_batchList->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Selection Error", "No student batch selected to delete.");
        return;
    }

    QString text = item->text();
    int idx = text.indexOf(" (Strength:");
    std::string name = (idx != -1) ? text.left(idx).toStdString() : text.toStdString();

    if (m_appManager.isBatchUsed(name)) {
        QMessageBox::critical(this, "Cannot Delete",
            "This batch is used in scheduled sessions and cannot be deleted.");
        return;
    }

    if (QMessageBox::question(this, "Confirm Delete",
            "Are you sure you want to delete this student batch?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    if (m_appManager.removeBatch(name)) {
        delete item;
        saveToFile();
        populateCombos();
        markConstraintsDirty();
    } else {
        QMessageBox::warning(this, "Delete Failed", "Unable to delete student batch.");
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// SESSION CRUD
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::onAddClassSession()
{
    bool isEditMode = !m_editingSessionId.isEmpty();

    QString instName   = m_sessInstCombo->currentText();
    QString courseCode = m_sessCourseCombo->currentData().toString();
    QString roomId     = m_sessRoomCombo->currentData().toString();
    QString batchId    = m_sessBatchCombo->currentData().toString();

    if (instName.isEmpty() || courseCode.isEmpty() || roomId.isEmpty() || batchId.isEmpty()) {
        QMessageBox::warning(this, "Selection Error",
            "Please ensure all entities (Instructors, Courses, Rooms, Batches) "
            "are created and selected.");
        return;
    }

    Instructor*   inst = m_appManager.findInstructorByName(instName.toStdString());
    if (!inst)    inst = m_appManager.findInstructorById(instName.toStdString());
    Course*       crs  = m_appManager.findCourseByCode(courseCode.toStdString());
    Room*         rm   = m_appManager.findRoomById(roomId.toStdString());
    StudentBatch* btch = m_appManager.findBatchById(batchId.toStdString());

    if (!inst || !crs || !rm || !btch) {
        QMessageBox::critical(this, "System Error",
            "Failed to retrieve references for the selected objects.");
        return;
    }

    // Subject qualification guard (applies in both Add and Edit modes)
    if (!inst->isQualifiedFor(crs->getCourseCode())) {
        QStringList lockedList;
        for (const auto& s : inst->getLockedSubjects())
            lockedList << QString::fromStdString(s);
        QMessageBox::warning(this, "Subject Not Assigned",
            QString("Instructor \"%1\" is not qualified to teach \"%2\".\n\n"
                    "Assigned subjects: %3")
                .arg(QString::fromStdString(inst->getName()))
                .arg(QString::fromStdString(crs->getCourseCode()))
                .arg(lockedList.isEmpty() ? "(none)" : lockedList.join(", ")));
        return;
    }

    QTime startTime = m_sessStartEdit->time();
    QTime endTime   = m_sessEndEdit->time();

    if (startTime >= endTime) {
        QMessageBox::warning(this, "Time Range Error",
            "End time must be strictly after start time.");
        return;
    }

    // Read current constraints for the time window check
    ConstraintSettings cs = readConstraintsFromUI();
    QTime csStart(cs.dayStartMinutes / 60, cs.dayStartMinutes % 60);
    QTime csEnd(cs.dayEndMinutes / 60, cs.dayEndMinutes % 60);

    if (startTime < csStart || endTime > csEnd) {
        QMessageBox::warning(this, "Time Range Error",
            QString("Classes must be scheduled between %1 and %2 (as set in Constraints tab).")
                .arg(csStart.toString("HH:mm"))
                .arg(csEnd.toString("HH:mm")));
        return;
    }

    // Determine Day from combo index
    // Combo order: Monday(0), Tuesday(1), Wednesday(2), Thursday(3), Friday(4), Sunday(5), Saturday(6)
    const Day dayMap[] = {
        Day::Monday, Day::Tuesday, Day::Wednesday, Day::Thursday,
        Day::Friday, Day::Sunday, Day::Saturday
    };
    Day day = dayMap[m_sessDayCombo->currentIndex()];

    ClockTime ctStart{ startTime.hour(), startTime.minute() };
    ClockTime ctEnd{   endTime.hour(),   endTime.minute()   };
    TimeSlot  slot(day, ctStart, ctEnd);

    // Soft constraint: 1-hr break after every 2 back-to-back classes for the batch.
    // In Edit mode, exclude the session being edited from the existing-slots list
    // (it will be replaced, so its old slot shouldn't count against the new one).
    std::vector<TimeSlot> batchDaySlots;
    batchDaySlots.push_back(slot);
    for (const auto& existing : m_appManager.getTimetable()) {
        if (isEditMode && existing.getSessionId() == m_editingSessionId.toStdString())
            continue; // skip self
        if (existing.getBatchId()->getBatchId() == btch->getBatchId() &&
            existing.getTimeSlot().getDay() == day)
            batchDaySlots.push_back(existing.getTimeSlot());
    }
    std::sort(batchDaySlots.begin(), batchDaySlots.end(), [](const TimeSlot& a, const TimeSlot& b) {
        return (a.getStartTime().hours * 60 + a.getStartTime().minutes) <
               (b.getStartTime().hours * 60 + b.getStartTime().minutes);
    });

    bool needsBreakWarning = false;
    int consecutiveClasses = 1;
    for (size_t i = 1; i < batchDaySlots.size(); ++i) {
        int prevEnd   = batchDaySlots[i-1].getEndTime().hours * 60 +
                        batchDaySlots[i-1].getEndTime().minutes;
        int currStart = batchDaySlots[i].getStartTime().hours * 60 +
                        batchDaySlots[i].getStartTime().minutes;
        if (currStart - prevEnd < 60) {
            ++consecutiveClasses;
            if (consecutiveClasses > 2) { needsBreakWarning = true; break; }
        } else {
            consecutiveClasses = 1;
        }
    }

    if (needsBreakWarning) {
        if (QMessageBox::question(this, "Batch Overload Warning",
                "This schedule places 3 or more classes for the student batch without a 1-hour break.\n\n"
                "Proceed anyway?",
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
    }

    if (isEditMode) {
        // ── Edit mode ──────────────────────────────────────────────────────────
        // Find the old session data for the workload check
        std::string oldInstId, oldCrsCode;
        for (const auto& sess : m_appManager.getTimetable()) {
            if (sess.getSessionId() == m_editingSessionId.toStdString()) {
                oldInstId  = sess.getTeacherId()->getId();
                oldCrsCode = sess.getSubjectId()->getCourseCode();
                break;
            }
        }

        bool instChanged = (oldInstId  != inst->getId());
        bool crsChanged  = (oldCrsCode != crs->getCourseCode());

        // Prospective workload check: only needed when instructor or course changes.
        if (instChanged || crsChanged) {
            int currentHours = m_appManager.countInstructorScheduledHours(inst->getId(), m_editingSessionId.toStdString());
            
            if (currentHours + 1 > inst->getMaxLimitHours()) {
                QMessageBox::warning(this, "Workload Limit Exceeded",
                    QString("Cannot save: \"%1\" would exceed the weekly hour limit for \"%2\".\n\n"
                            "Max Weekly Limit: %3 hours\n"
                            "Assigned so far: %4 hours\n")
                    .arg(QString::fromStdString(crs->getCourseCode()))
                    .arg(QString::fromStdString(inst->getName()))
                    .arg(inst->getMaxLimitHours())
                    .arg(currentHours));
                return;
            }
        }

        // Build the updated session — pass the existing sessionId so it is preserved.
        ClassSession updatedSession(slot, inst, crs, rm, btch,
                                    m_editingSessionId.toStdString());

        std::string err = m_appManager.validateAndUpdateClassSession(
            m_editingSessionId.toStdString(), updatedSession, cs);

        if (!err.empty()) {
            QMessageBox::warning(this, "Scheduling Constraint Violation",
                QString::fromStdString(err));
            return;
        }

        saveToFile();
        refreshListsAndTables();
        QMessageBox::information(this, "Save Succeeded",
            "Class session updated successfully!");

        if (m_addSessionDialog) {
            m_addSessionDialog->accept();
        }

    } else {
        // ── Add mode (original behavior, unchanged) ────────────────────────────
        int currentHours = m_appManager.countInstructorScheduledHours(inst->getId());
        if (currentHours + 1 > inst->getMaxLimitHours()) {
            QMessageBox::warning(this, "Workload Limit Exceeded",
                QString("Cannot schedule: %1 would exceed weekly hour limit!\n\n"
                        "Instructor: %2\nMax Weekly Limit: %3 hours\n"
                        "Assigned so far: %4 hours\n")
                .arg(QString::fromStdString(crs->getCourseCode()))
                .arg(QString::fromStdString(inst->getName()))
                .arg(inst->getMaxLimitHours())
                .arg(currentHours));
            return;
        }

        ClassSession session(slot, inst, crs, rm, btch);
        std::string err = m_appManager.validateAndAddClassSession(session, cs);
        if (!err.empty()) {
            QMessageBox::warning(this, "Scheduling Constraint Violation",
                QString::fromStdString(err));
            return;
        }

        saveToFile();
        refreshListsAndTables();
        QMessageBox::information(this, "Schedule Succeeded",
            "Class session scheduled successfully!");

        if (m_addSessionDialog) {
            m_addSessionDialog->accept();
        }
    }
}


void MainWindow::onDeleteClassSession()
{
    std::string sessionId;

    if (m_timetableSubTabs->currentIndex() == 0) {
        // We are on the Schedule (Table) view
        int row = m_timetableTable->currentRow();
        if (row < 0) {
            QMessageBox::warning(this, "Selection Error",
                "No class session selected in the Schedule table to delete.");
            return;
        }

        QTableWidgetItem* item = m_timetableTable->item(row, 0);
        if (!item) return;
        sessionId = item->data(Qt::UserRole).toString().toStdString();
    } else {
        // We are on the Grid View
        int row = m_timetableGrid->currentRow();
        int col = m_timetableGrid->currentColumn();
        if (row < 0 || col < 0) {
            QMessageBox::warning(this, "Selection Error",
                "No class session selected in the Grid View to delete.");
            return;
        }

        QTableWidgetItem* item = m_timetableGrid->item(row, col);
        if (!item || item->data(Qt::UserRole).isNull() || item->data(Qt::UserRole).toString().isEmpty()) {
            QMessageBox::warning(this, "Selection Error",
                "Please select a valid scheduled session block in the Grid View.");
            return;
        }
        sessionId = item->data(Qt::UserRole).toString().toStdString();
    }

    if (QMessageBox::question(this, "Confirm Delete",
            "Are you sure you want to remove this scheduled session?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    if (m_appManager.removeClassSession(sessionId)) {
        saveToFile();
        refreshListsAndTables();
    } else {
        QMessageBox::warning(this, "Delete Failed",
            "Unable to remove the selected session.");
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// onAutoGenerate — gated by constraintsValidated
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::onAutoGenerate()
{
    if (!m_constraintsValidated) {
        QMessageBox::warning(this, "Constraints Not Validated",
            "Please validate constraints first before generating the routine.\n\n"
            "Go to the Constraints tab and click \"Validate Constraints\".");
        return;
    }

    ConstraintSettings cs = readConstraintsFromUI();

    if (QMessageBox::question(this, "Auto Generate Routine",
            "This will clear the current timetable and generate a new routine "
            "based on your configured constraints.\n\nProceed?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    std::string genError = m_appManager.autoGenerateTimetable(cs);
    if (!genError.empty()) {
        QMessageBox::warning(this, "Generation Failed", QString::fromStdString(genError));
        // We still save to file so any partial changes/cleared timetables are synced?
        // Actually, returning early without saving leaves the UI empty. Better to refresh UI
        // and show the error.
        saveToFile();
        refreshListsAndTables();
        return;
    }

    // Post-generation validation: Ensure no invalid/truncated course codes bypassed generation constraints
    bool dataCorrupted = false;
    for (const auto& session : m_appManager.getTimetable()) {
        std::string genCode = session.getSubjectId()->getCourseCode();
        if (!m_appManager.findCourseByCode(genCode)) {
            dataCorrupted = true;
            break;
        }
    }
    
    if (dataCorrupted) {
        QMessageBox::critical(this, "Data Corruption Detected",
            "A session with an invalid or truncated Course Code was detected during generation.\n"
            "This usually indicates that a course was not retrieved or stored properly. "
            "Please clear the timetable and verify your course assignments.");
        // We still save/refresh to allow the user to see the issue if they wish, 
        // but we alerted them. Alternatively, we could clear it. We will just alert for now.
    }

    saveToFile();
    refreshListsAndTables();

    // Count how many working days were used
    int workingDayCount = 0;
    for (int i = 0; i < 7; ++i) if (cs.workingDays[i]) ++workingDayCount;

    int total = static_cast<int>(m_appManager.getTimetable().size());
    int startH = cs.dayStartMinutes / 60, startM = cs.dayStartMinutes % 60;
    int endH   = cs.dayEndMinutes   / 60, endM   = cs.dayEndMinutes   % 60;

    QString lunchInfo = cs.lunchBreakEnabled
        ? QString("Lunch break: %1:%2 – %3:%4")
              .arg(cs.lunchStartMinutes/60,2,10,QChar('0'))
              .arg(cs.lunchStartMinutes%60,2,10,QChar('0'))
              .arg(cs.lunchEndMinutes/60,2,10,QChar('0'))
              .arg(cs.lunchEndMinutes%60,2,10,QChar('0'))
        : "No lunch break";

    QMessageBox::information(this, "Generation Complete",
        QString("Routine generated successfully!\n\n"
                "Total sessions scheduled: %1\n"
                "Working days: %2\n"
                "Time window: %3:%4 – %5:%6\n"
                "%7")
        .arg(total)
        .arg(workingDayCount)
        .arg(startH,2,10,QChar('0')).arg(startM,2,10,QChar('0'))
        .arg(endH,2,10,QChar('0')).arg(endM,2,10,QChar('0'))
        .arg(lunchInfo));
}
