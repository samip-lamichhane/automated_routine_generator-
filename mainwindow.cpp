#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Instructor.hpp"
#include "Course.hpp"
#include "room.hpp"
#include "Student_batch.hpp"
#include "classSession.hpp"
#include "timeslot.hpp"
#include "AppManager.hpp"
#include "CRUD.h"
#include "schedule_validator.hpp"
#include "mainwindow_helper.hpp"

#include <QHeaderView>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QMenu>
#include <QAction>
#include <QSplitter>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QScrollArea>
#include <algorithm>

// ──────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ──────────────────────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_crudManager(new CRUDManager(this))
{
    ui->setupUi(this);

    // ── Wire up the day-checkbox array for loop access ────────────────────────
    m_dayChecks[0] = ui->dayCheck0;  // Sun
    m_dayChecks[1] = ui->dayCheck1;  // Mon
    m_dayChecks[2] = ui->dayCheck2;  // Tue
    m_dayChecks[3] = ui->dayCheck3;  // Wed
    m_dayChecks[4] = ui->dayCheck4;  // Thu
    m_dayChecks[5] = ui->dayCheck5;  // Fri
    m_dayChecks[6] = ui->dayCheck6;  // Sat

    // ── Grab the subject layout (created by ui->setupUi) ─────────
    m_instSubjectLayout = ui->instSubjectLayout;

    // ── Global stylesheet (Catppuccin-inspired warm theme) ───────────────────
    MainWindowHelper::applyGlobalStyle(this);

    applyDynamicStyles();
    setupSessionDialog();
    connectSignals();

    loadFromFile();
    refreshListsAndTables();
    populateCombos();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ──────────────────────────────────────────────────────────────────────────────
// applyDynamicStyles — per-widget stylesheets applied after setupUi
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::applyDynamicStyles()
{
    MainWindowHelper::applyDynamicStyles(this, ui, m_dayChecks);

    // Initial capacity label computation will happen after loadFromFile / connectSignals
    updateCapacityLabel();
}

// ──────────────────────────────────────────────────────────────────────────────
// setupSessionDialog — builds the Add/Edit Class Session dialog
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::setupSessionDialog()
{
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
            background-color: #6B3654;
            color: #E8DDC7;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #55293F; }
    )");
    m_btnDialogDelete->setVisible(false); // Only shown in Edit mode

    dialogBtnLayout->addWidget(m_btnDialogDelete);
    dialogBtnLayout->addStretch();
    dialogBtnLayout->addWidget(btnDialogCancel);
    dialogBtnLayout->addWidget(m_btnDialogSchedule);
    dialogLayout->addLayout(dialogBtnLayout);

    connect(m_btnDialogSchedule, &QPushButton::clicked, m_crudManager, &CRUDManager::onAddClassSession);
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
}

// ──────────────────────────────────────────────────────────────────────────────
// connectSignals — all signal/slot connections for ui widgets
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::connectSignals()
{
    // ── Instructors tab ───────────────────────────────────────────────────────
    connect(ui->instSubjectCountSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onSubjectCountChanged);
    connect(ui->btnInstAdd,    &QPushButton::clicked, m_crudManager, &CRUDManager::onAddInstructor);
    connect(ui->btnInstEdit,   &QPushButton::clicked, m_crudManager, &CRUDManager::onEditInstructor);
    connect(ui->btnInstDelete, &QPushButton::clicked, m_crudManager, &CRUDManager::onDeleteInstructor);

    // ── Courses tab ───────────────────────────────────────────────────────────
    connect(ui->btnCourseAdd,    &QPushButton::clicked, m_crudManager, &CRUDManager::onAddCourse);
    connect(ui->btnCourseEdit,   &QPushButton::clicked, m_crudManager, &CRUDManager::onEditCourse);
    connect(ui->btnCourseDelete, &QPushButton::clicked, m_crudManager, &CRUDManager::onDeleteCourse);

    // ── Rooms tab ─────────────────────────────────────────────────────────────
    connect(ui->btnRoomAdd,    &QPushButton::clicked, m_crudManager, &CRUDManager::onAddRoom);
    connect(ui->btnRoomEdit,   &QPushButton::clicked, m_crudManager, &CRUDManager::onEditRoom);
    connect(ui->btnRoomDelete, &QPushButton::clicked, m_crudManager, &CRUDManager::onDeleteRoom);

    // ── Student Batches tab ───────────────────────────────────────────────────
    connect(ui->btnBatchAdd,    &QPushButton::clicked, m_crudManager, &CRUDManager::onAddBatch);
    connect(ui->btnBatchEdit,   &QPushButton::clicked, m_crudManager, &CRUDManager::onEditBatch);
    connect(ui->btnBatchDelete, &QPushButton::clicked, m_crudManager, &CRUDManager::onDeleteBatch);

    // ── Constraints tab ───────────────────────────────────────────────────────
    for (int i = 0; i < 7; ++i)
        connect(m_dayChecks[i], &QCheckBox::toggled, this, &MainWindow::onConstraintsChanged);

    connect(ui->dayStartEdit,        &QTimeEdit::timeChanged, this, &MainWindow::onConstraintsChanged);
    connect(ui->dayEndEdit,          &QTimeEdit::timeChanged, this, &MainWindow::onConstraintsChanged);
    connect(ui->lunchEnabledCheck,   &QCheckBox::toggled,     this, &MainWindow::onConstraintsChanged);
    connect(ui->lunchStartEdit,      &QTimeEdit::timeChanged, this, &MainWindow::onConstraintsChanged);
    connect(ui->lunchEndEdit,        &QTimeEdit::timeChanged, this, &MainWindow::onConstraintsChanged);

    connect(ui->ruleNoInstDoubleBook,    &QCheckBox::toggled, this, &MainWindow::onConstraintsChanged);
    connect(ui->ruleNoRoomDoubleBook,    &QCheckBox::toggled, this, &MainWindow::onConstraintsChanged);
    connect(ui->ruleNoBatchClash,        &QCheckBox::toggled, this, &MainWindow::onConstraintsChanged);
    connect(ui->ruleInstDayGap,          &QCheckBox::toggled, this, &MainWindow::onConstraintsChanged);
    connect(ui->ruleNoSameSubjectConsec, &QCheckBox::toggled, this, &MainWindow::onConstraintsChanged);
    connect(ui->ruleMaxWeeklyHours,      &QCheckBox::toggled, this, &MainWindow::onConstraintsChanged);
    connect(ui->ruleMaxConsecHoursEnabled, &QCheckBox::toggled, this, &MainWindow::onConstraintsChanged);
    connect(ui->maxConsecHoursSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onConstraintsChanged);

    connect(ui->btnValidate, &QPushButton::clicked, this, &MainWindow::onValidateConstraints);

    // ── Timetable tab ─────────────────────────────────────────────────────────
    connect(ui->btnOpenAddDialog, &QPushButton::clicked, this, [this]() {
        openSessionDialogForAdd();
    });
    connect(ui->btnSessionDelete,  &QPushButton::clicked, m_crudManager, &CRUDManager::onDeleteClassSession);
    connect(ui->btnAutoGenerate,   &QPushButton::clicked, this, &MainWindow::onAutoGenerate);
    connect(ui->btnResetAllData,   &QPushButton::clicked, m_crudManager, &CRUDManager::onResetAllData);

    connect(ui->viewBatchCombo, &QComboBox::currentIndexChanged, this, &MainWindow::onViewBatchChanged);
    connect(ui->btnRefreshGrid,  &QPushButton::clicked, this, &MainWindow::onRefreshGridClicked);

    // Grid cell click for edit-in-place
    connect(ui->timetableGrid, &QTableWidget::cellClicked,
            this, &MainWindow::onGridCellClicked);
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

    QTime start = ui->dayStartEdit->time();
    QTime end   = ui->dayEndEdit->time();
    cs.dayStartMinutes = start.hour() * 60 + start.minute();
    cs.dayEndMinutes   = end.hour()   * 60 + end.minute();

    cs.lunchBreakEnabled = ui->lunchEnabledCheck->isChecked();
    QTime ls = ui->lunchStartEdit->time();
    QTime le = ui->lunchEndEdit->time();
    cs.lunchStartMinutes = ls.hour() * 60 + ls.minute();
    cs.lunchEndMinutes   = le.hour() * 60 + le.minute();

    cs.ruleNoInstructorDoubleBook  = ui->ruleNoInstDoubleBook->isChecked();
    cs.ruleNoRoomDoubleBook        = ui->ruleNoRoomDoubleBook->isChecked();
    cs.ruleNoBatchClash            = ui->ruleNoBatchClash->isChecked();
    cs.ruleInstructorDayGap        = ui->ruleInstDayGap->isChecked();
    cs.ruleNoSameSubjectConsecDays = ui->ruleNoSameSubjectConsec->isChecked();
    cs.ruleRespectMaxWeeklyHours   = ui->ruleMaxWeeklyHours->isChecked();
    cs.ruleMaxConsecHoursEnabled   = ui->ruleMaxConsecHoursEnabled->isChecked();
    cs.ruleMaxConsecHoursPerDay    = ui->maxConsecHoursSpin->value();
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

    ui->capacityLabel->setText(
        QString("Available capacity:  %1 hrs/week  (%2 working day%3 × %4 hrs/day)")
            .arg(weeklyHours, 0, 'f', 1)
            .arg(workingDayCount)
            .arg(workingDayCount != 1 ? "s" : "")
            .arg(dailyMinutes / 60.0, 0, 'f', 1));
}

void MainWindow::markConstraintsDirty()
{
    m_constraintsValidated = false;
    if (ui->btnAutoGenerate) {
        ui->btnAutoGenerate->setEnabled(false);
        ui->btnAutoGenerate->setToolTip(
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

    std::string outLog;
    bool allPassed = ScheduleValidator::validateFeasibility(m_appManager, cs, outLog);

    QString output = QString::fromStdString(outLog);

    // ── Final verdict ───────────────────────────────────────────────────────
    output += "<br>";
    if (allPassed) {
        output += "<b style='color:#5A7A4A; font-size:14px'>"
                  "✔  All checks passed — you may now Auto Generate the timetable.</b>";
        m_constraintsValidated = true;
        m_constraints = cs;
        ui->btnAutoGenerate->setEnabled(true);
        ui->btnAutoGenerate->setToolTip("Constraints validated — ready to generate.");
    } else {
        output += "<b style='color:#C25B3A; font-size:14px'>"
                  "✘  Some checks failed — please fix the issues above before generating.</b>";
        m_constraintsValidated = false;
        ui->btnAutoGenerate->setEnabled(false);
    }

    ui->validationOutput->setHtml(output);
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

void MainWindow::refreshInstList()
{
    ui->instList->clear();
    for (const auto& inst : m_appManager.getInstructors())
        ui->instList->addItem(MainWindowHelper::instDisplayString(inst));
}

// ──────────────────────────────────────────────────────────────────────────────
// populateInitialData
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::populateInitialData()
{
    MainWindowHelper::populateInitialData(m_appManager);

    refreshInstList();
    ui->courseList->addItem("COMP-102 (Allocated Hours: 3)");
    ui->courseList->addItem("MATH-101 (Allocated Hours: 4)");
    ui->roomList->addItem("Block-C-102 (Capacity: 60, Type: Theory)");
    ui->roomList->addItem("Lab-A-301 (Capacity: 40, Type: Lab)");
    ui->batchList->addItem("BCT-2025-A (Strength: 48, Program: BCE)");
    ui->batchList->addItem("BIT-2025-B (Strength: 45, Program: BIT)");
}

// ──────────────────────────────────────────────────────────────────────────────
// saveToFile
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::saveToFile()
{
    QString filePath = QCoreApplication::applicationDirPath() + "/timetable_data.json";
    MainWindowHelper::saveTimetableData(filePath, m_appManager, readConstraintsFromUI());
}

// ──────────────────────────────────────────────────────────────────────────────
// loadFromFile
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::loadFromFile()
{
    QString filePath = QCoreApplication::applicationDirPath() + "/timetable_data.json";
    bool loaded = false;
    QJsonObject csObj = MainWindowHelper::loadTimetableData(filePath, m_appManager, loaded);

    if (!loaded) {
        populateInitialData();
        saveToFile();
        return;
    }

    if (!csObj.isEmpty()) {
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
        loadTime("dayStartMinutes",   ui->dayStartEdit);
        loadTime("dayEndMinutes",     ui->dayEndEdit);
        loadTime("lunchStartMinutes", ui->lunchStartEdit);
        loadTime("lunchEndMinutes",   ui->lunchEndEdit);

        if (csObj.contains("lunchBreakEnabled"))
            ui->lunchEnabledCheck->setChecked(csObj["lunchBreakEnabled"].toBool());

        auto loadRule = [&](const QString& key, QCheckBox* chk) {
            if (csObj.contains(key)) chk->setChecked(csObj[key].toBool());
        };
        loadRule("ruleNoInstDoubleBook", ui->ruleNoInstDoubleBook);
        loadRule("ruleNoRoomDoubleBook", ui->ruleNoRoomDoubleBook);
        loadRule("ruleNoBatchClash",     ui->ruleNoBatchClash);
        loadRule("ruleInstDayGap",       ui->ruleInstDayGap);
        loadRule("ruleNoSameSubjConsec", ui->ruleNoSameSubjectConsec);
        loadRule("ruleMaxWeeklyHours",   ui->ruleMaxWeeklyHours);
        loadRule("ruleMaxConsecEnabled", ui->ruleMaxConsecHoursEnabled);

        if (csObj.contains("ruleMaxConsecHours"))
            ui->maxConsecHoursSpin->setValue(csObj["ruleMaxConsecHours"].toInt());
    }

    // Rebuild list widgets
    refreshInstList();
    ui->courseList->clear();
    for (const auto& crs : m_appManager.getCourses()) {
        ui->courseList->addItem(QString("%1 (Allocated Hours: %2)")
            .arg(QString::fromStdString(crs.getCourseCode()))
            .arg(crs.getAllocatedHours()));
    }
    ui->roomList->clear();
    for (const auto& rm : m_appManager.getRooms()) {
        ui->roomList->addItem(QString("%1 (Capacity: %2, Type: %3)")
            .arg(QString::fromStdString(rm.getRoomId()))
            .arg(rm.getCapacity())
            .arg(QString::fromStdString(rm.getTypeAsString())));
    }
    ui->batchList->clear();
    for (const auto& b : m_appManager.getBatches()) {
        ui->batchList->addItem(QString("%1 (Strength: %2, Program: %3)")
            .arg(QString::fromStdString(b.getBatchId()))
            .arg(b.getStrength())
            .arg(QString::fromStdString(b.getProgramAsString())));
    }

    rebuildSubjectCombos(ui->instSubjectCountSpin->value());
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

    ui->viewBatchCombo->clear();
    ui->viewBatchCombo->addItem("-- Select Batch to View --", QVariant(""));

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
        ui->viewBatchCombo->addItem(QString::fromStdString(batch.getBatchId()),
                                  QVariant(QString::fromStdString(batch.getBatchId())));
    }

    rebuildSubjectCombos(ui->instSubjectCountSpin->value());
}

// ──────────────────────────────────────────────────────────────────────────────
// refreshListsAndTables
// ──────────────────────────────────────────────────────────────────────────────

void MainWindow::onViewBatchChanged()
{
    refreshTimetableGrid();
}

void MainWindow::refreshListsAndTables()
{
    const auto& timetable = m_appManager.getTimetable();
    ui->timetableTable->setRowCount(0);
    for (const auto& session : timetable) {
        int row = ui->timetableTable->rowCount();
        ui->timetableTable->insertRow(row);

        TimeSlot ts = session.getTimeSlot();
        QString timeStr = MainWindowHelper::formatClockTime(ts.getStartTime()) + " - " + MainWindowHelper::formatClockTime(ts.getEndTime());

        QTableWidgetItem* dayItem = new QTableWidgetItem(MainWindowHelper::dayToString(ts.getDay()));
        dayItem->setData(Qt::UserRole, QString::fromStdString(session.getSessionId()));
        ui->timetableTable->setItem(row, 0, dayItem);
        ui->timetableTable->setItem(row, 1, new QTableWidgetItem(timeStr));
        ui->timetableTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(session.getSubjectId()->getCourseCode())));
        ui->timetableTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(session.getTeacherId()->getName())));
        ui->timetableTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(session.getRoomId()->getRoomId())));
        ui->timetableTable->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(session.getBatchId()->getBatchId())));
        ui->timetableTable->setItem(row, 6, new QTableWidgetItem(QString("%1 mins").arg(ts.getDurationmin())));
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
    QTableWidgetItem *item = ui->timetableGrid->item(row, col);
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
        QTableWidgetItem *vHeader = ui->timetableGrid->verticalHeaderItem(prefillDayRow);
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
        QTableWidgetItem *hHeader = ui->timetableGrid->horizontalHeaderItem(prefillColSlot);
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
    QString batchId = ui->viewBatchCombo->currentData().toString();
    MainWindowHelper::refreshTimetableGrid(ui, m_appManager, readConstraintsFromUI(), batchId);
}
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
        saveToFile();
        refreshListsAndTables();
        return;
    }

    // Post-generation validation: Ensure no invalid/truncated course codes bypassed generation constraints
    bool dataCorrupted = ScheduleValidator::isDataCorrupted(m_appManager);

    if (dataCorrupted) {
        QMessageBox::critical(this, "Data Corruption Detected",
            "A session with an invalid or truncated Course Code was detected during generation.\n"
            "This usually indicates that a course was not retrieved or stored properly. "
            "Please clear the timetable and verify your course assignments.");
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

