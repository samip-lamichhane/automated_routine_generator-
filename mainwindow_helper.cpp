#include "mainwindow_helper.hpp"
#include "ui_mainwindow.h"
#include "AppManager.hpp"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QHeaderView>
#include <QLabel>
#include <algorithm>
#include <QMainWindow>

namespace MainWindowHelper {

QString dayToString(Day d) {
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

QString formatClockTime(ClockTime t) {
    return QString("%1:%2")
        .arg(t.hours,   2, 10, QChar('0'))
        .arg(t.minutes, 2, 10, QChar('0'));
}

QString instDisplayString(const Instructor& inst) {
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

QMap<QString, QColor> buildCourseColorMap(const std::vector<Course>& courses) {
    QMap<QString, QColor> colorMap;
    const std::vector<std::string> palette = {
        "#DDB1BB", "#BDCDA9", "#E8B9AA", "#9DCBCA", "#A8C6EA",
        "#D7ADC1", "#C8D8B6", "#E8C1AB", "#ADC8D8", "#E1B1C7"
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

void populateInitialData(AppManager& appManager) {
    appManager.addCourse(Course("COMP-102", "Computer Programming", 3));
    appManager.addCourse(Course("MATH-101", "Mathematics I", 4));

    {
        Instructor inst("Dr. Niraj Sharma", 12);
        inst.setLockedSubjects({"COMP-102"});
        appManager.addInstructor(inst);
    }
    {
        Instructor inst("Prof. Ram Prasad", 15);
        inst.setLockedSubjects({"MATH-101"});
        appManager.addInstructor(inst);
    }

    appManager.addRoom(Room("Block-C-102", 60, RoomType::Theory));
    appManager.addRoom(Room("Lab-A-301",   40, RoomType::Lab));

    appManager.addBatch(StudentBatch("BCT-2025-A", 48, ProgramType::BCE));
    appManager.addBatch(StudentBatch("BIT-2025-B", 45, ProgramType::BIT));
}

void applyGlobalStyle(QMainWindow* window) {
    window->setStyleSheet(R"(
        QMainWindow {
            background-color: #E8DDC7;
        }
        QMessageBox {
            background-color: #E8DDC7;
        }
        QDialog {
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
}

void applyDynamicStyles(QMainWindow* window, Ui::MainWindow* ui, QCheckBox** dayChecks) {
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
    for (int i = 0; i < 7; ++i)
        dayChecks[i]->setStyleSheet(chipStyle);

    ui->capacityLabel->setStyleSheet(
        "color: #A83A28; font-weight: bold; font-size: 13px; "
        "padding: 6px 10px; background-color: #E8DDC7; "
        "border: 1px solid #5A1810; border-radius: 6px;");

    ui->validationOutput->setStyleSheet(
        "QTextEdit {"
        "  background-color: #E8DDC7;"
        "  color: #2A1A12;"
        "  border: 1px solid #5A1810;"
        "  border-radius: 8px;"
        "  padding: 5px;"
        "  font-family: 'Segoe UI', Helvetica, sans-serif;"
        "}"
    );

    ui->timetableSubTabs->setStyleSheet(R"(
        QTabWidget::pane { border: 1px solid #5A1810; background: #E8DDC7; }
        QTabBar::tab { background: #E8DDC7; color: #6B5D48; padding: 8px 16px; border: 1px solid #5A1810; }
        QTabBar::tab:selected { background: #E8DDC7; color: #A83A28; border-bottom-color: #E8DDC7; font-weight: bold; }
    )");

    ui->timetableGrid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->timetableGrid->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->timetableGrid->verticalHeader()->setMinimumSectionSize(115);
    ui->timetableGrid->verticalHeader()->setMinimumWidth(80);
    ui->timetableGrid->setStyleSheet(R"(
        QTableWidget {
            gridline-color: #E8DDC7;
            border: 2px solid #5A1810;
            border-radius: 8px;
            background-color: #E8DDC7;
        }
        QHeaderView::section:horizontal {
            background-color: #6E6E6E;
            color: #E8DDC7;
            font-weight: 500;
            font-size: 11px;
            padding: 4px;
            border: none;
        }
        QHeaderView::section:vertical {
            background-color: #6E6E6E;
            color: #E8DDC7;
            font-weight: bold;
            font-size: 11px;
            padding: 4px;
            border: none;
        }
        QTableWidget QTableCornerButton::section {
            background-color: #6E6E6E;
        }
        QTableWidget::item {
            padding: 4px;
        }
    )");

    ui->viewBatchCombo->setStyleSheet(R"(
        QComboBox { background-color: #D9C9A8; color: #A83A28; border: none; border-radius: 8px; padding: 4px 12px; font-weight: bold; }
        QComboBox::drop-down { border: none; }
        QComboBox:hover { background-color: #5A1810; }
    )");

    ui->btnRefreshGrid->setStyleSheet(R"(
        QPushButton { background-color: #D9C9A8; color: #A83A28; border: none; border-radius: 8px; padding: 6px 12px; font-weight: bold; }
        QPushButton:hover { background-color: #5A1810; }
    )");

    ui->btnOpenAddDialog->setStyleSheet(R"(
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

    ui->btnAutoGenerate->setStyleSheet(R"(
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

    ui->btnSessionDelete->setStyleSheet(R"(
        QPushButton {
            background-color: #6B3654;
            color: #E8DDC7;
            border: none;
            border-radius: 8px;
            padding: 12px 20px;
            font-weight: bold;
            font-size: 15px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QPushButton:hover { background-color: #55293F; }
    )");

    ui->btnResetAllData->setStyleSheet(R"(
        QPushButton {
            background-color: #B22222;
            color: #FFFFFF;
            border: none;
            border-radius: 8px;
            padding: 12px 20px;
            font-weight: bold;
            font-size: 15px;
            font-family: 'Segoe UI', Helvetica, sans-serif;
        }
        QPushButton:hover { background-color: #8B0000; }
    )");

    ui->constraintsInner->setObjectName("constraintsInner");
    ui->constraintsInner->setStyleSheet("#constraintsInner { background-color: transparent; }");

    ui->constraintsSplitter->setStretchFactor(0, 3);
    ui->constraintsSplitter->setStretchFactor(1, 2);
}

void refreshTimetableGrid(Ui::MainWindow* ui, const AppManager& appManager, const ConstraintSettings& cs, const QString& batchId) {
    ui->timetableGrid->clear();
    ui->timetableGrid->clearSpans();

    if (batchId.isEmpty()) {
        ui->timetableGrid->setRowCount(0);
        ui->timetableGrid->setColumnCount(0);
        return;
    }

    QMap<QString, QColor> courseColors = buildCourseColorMap(appManager.getCourses());

    std::vector<Day> workDays;
    for (int i = 0; i < 7; ++i) {
        if (cs.workingDays[i]) workDays.push_back(static_cast<Day>(i));
    }

    ui->timetableGrid->setRowCount(workDays.size());
    QStringList vHeaders;
    for (Day d : workDays) vHeaders << dayToString(d);
    ui->timetableGrid->setVerticalHeaderLabels(vHeaders);

    int totalMinutes = cs.dayEndMinutes - cs.dayStartMinutes;
    int cols = totalMinutes / 60;
    if (cols < 0) cols = 0;
    ui->timetableGrid->setColumnCount(cols);

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
    ui->timetableGrid->setHorizontalHeaderLabels(hHeaders);

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
            ui->timetableGrid->setItem(r, c, item);
        }
    }

    // Populate actual sessions
    std::vector<ClassSession> batchSessions;
    for (const auto& session : appManager.getTimetable()) {
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
        QColor bg = courseColors.value(cCode, QColor("#ADC8D8"));
        item->setBackground(bg);
        item->setData(Qt::UserRole, QVariant(sessionIds.join(",")));

        ui->timetableGrid->setItem(r, startSlot, item);

        QLabel *lbl = new QLabel(cellHtml);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setWordWrap(true);
        lbl->setAttribute(Qt::WA_TransparentForMouseEvents);
        lbl->setStyleSheet("background: transparent;");
        ui->timetableGrid->setCellWidget(r, startSlot, lbl);

        if (totalSpanSlots > 1) {
            ui->timetableGrid->setSpan(r, startSlot, 1, totalSpanSlots);
        }
    }
}

void saveTimetableData(const QString& filePath, const AppManager& appManager, const ConstraintSettings& cs) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }

    QJsonObject rootObj;

    QJsonArray instArray;
    for (const auto& inst : appManager.getInstructors()) {
        QJsonObject instObj;
        instObj["id"]            = QString::fromStdString(inst.getId());
        instObj["name"]          = QString::fromStdString(inst.getName());
        instObj["maxLimitHours"] = inst.getMaxLimitHours();

        QJsonArray lockedArray;
        for (const auto& s : inst.getLockedSubjects())
            lockedArray.append(QString::fromStdString(s));
        instObj["lockedSubjects"] = lockedArray;
        instObj["assignedCourses"] = QJsonArray();

        instArray.append(instObj);
    }
    rootObj["instructors"] = instArray;

    QJsonArray courseArray;
    for (const auto& crs : appManager.getCourses()) {
        QJsonObject crsObj;
        crsObj["code"]           = QString::fromStdString(crs.getCode());
        crsObj["name"]           = QString::fromStdString(crs.getName());
        crsObj["creditHours"]    = crs.getCreditHours();
        crsObj["courseCode"]     = QString::fromStdString(crs.getCourseCode());
        crsObj["allocatedHours"] = crs.getAllocatedHours();
        courseArray.append(crsObj);
    }
    rootObj["courses"] = courseArray;

    QJsonArray roomArray;
    for (const auto& rm : appManager.getRooms()) {
        QJsonObject rmObj;
        rmObj["roomId"]   = QString::fromStdString(rm.getRoomId());
        rmObj["capacity"] = rm.getCapacity();
        rmObj["type"]     = static_cast<int>(rm.getType());
        rmObj["building"] = QString::fromStdString(rm.getBuilding());
        roomArray.append(rmObj);
    }
    rootObj["rooms"] = roomArray;

    QJsonArray batchArray;
    for (const auto& b : appManager.getBatches()) {
        QJsonObject bObj;
        bObj["batchId"]    = QString::fromStdString(b.getBatchId());
        bObj["strength"]   = b.getStrength();
        bObj["program"]    = static_cast<int>(b.getProgram());
        bObj["department"] = QString::fromStdString(b.getDepartment());
        batchArray.append(bObj);
    }
    rootObj["batches"] = batchArray;

    QJsonArray sessionArray;
    for (const auto& s : appManager.getTimetable()) {
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

    QJsonObject csObj;
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

QJsonObject loadTimetableData(const QString& filePath, AppManager& appManager, bool& outLoaded) {
    outLoaded = false;
    QJsonObject emptyObj;

    if (!QFile::exists(filePath)) return emptyObj;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return emptyObj;

    QByteArray saveData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(saveData);
    if (doc.isNull() || !doc.isObject()) return emptyObj;

    QJsonObject rootObj = doc.object();

    if (rootObj.contains("courses") && rootObj["courses"].isArray()) {
        for (const auto& val : rootObj["courses"].toArray()) {
            QJsonObject crsObj = val.toObject();
            std::string code = crsObj.contains("code") ? crsObj["code"].toString().toStdString() : crsObj["courseCode"].toString().toStdString();
            std::string name = crsObj.contains("name") ? crsObj["name"].toString().toStdString() : code;
            int creditHours  = crsObj.contains("creditHours") ? crsObj["creditHours"].toInt() : crsObj["allocatedHours"].toInt();
            appManager.addCourse(Course(code, name, creditHours));
        }
    }

    if (rootObj.contains("instructors") && rootObj["instructors"].isArray()) {
        for (const auto& val : rootObj["instructors"].toArray()) {
            QJsonObject instObj = val.toObject();
            std::string id   = instObj.contains("id") ? instObj["id"].toString().toStdString() : instObj["name"].toString().toStdString();
            std::string name = instObj["name"].toString().toStdString();
            int maxHours     = instObj["maxLimitHours"].toInt();
            Instructor inst(id, name, maxHours);

            if (instObj.contains("lockedSubjects") && instObj["lockedSubjects"].isArray()) {
                std::vector<std::string> locked;
                for (const auto& lv : instObj["lockedSubjects"].toArray())
                    locked.push_back(lv.toString().toStdString());
                inst.setLockedSubjects(locked);
            }
            appManager.addInstructor(inst);
        }
    }

    if (rootObj.contains("rooms") && rootObj["rooms"].isArray()) {
        for (const auto& val : rootObj["rooms"].toArray()) {
            QJsonObject rmObj = val.toObject();
            std::string id    = rmObj["roomId"].toString().toStdString();
            int cap           = rmObj["capacity"].toInt();
            RoomType type     = static_cast<RoomType>(rmObj["type"].toInt());
            std::string building = rmObj.contains("building") ? rmObj["building"].toString().toStdString() : "Main";
            appManager.addRoom(Room(id, cap, type, building));
        }
    }

    if (rootObj.contains("batches") && rootObj["batches"].isArray()) {
        for (const auto& val : rootObj["batches"].toArray()) {
            QJsonObject bObj = val.toObject();
            std::string id   = bObj["batchId"].toString().toStdString();
            int strength     = bObj["strength"].toInt();
            ProgramType prog = static_cast<ProgramType>(bObj["program"].toInt());
            std::string dept = bObj.contains("department") ? bObj["department"].toString().toStdString() : "CS";
            appManager.addBatch(StudentBatch(id, strength, prog, dept));
        }
    }

    if (rootObj.contains("timetable") && rootObj["timetable"].isArray()) {
        for (const auto& val : rootObj["timetable"].toArray()) {
            QJsonObject sObj = val.toObject();
            Day day = static_cast<Day>(sObj["day"].toInt());
            ClockTime ctStart{ sObj["startH"].toInt(), sObj["startM"].toInt() };
            ClockTime ctEnd{   sObj["endH"].toInt(),   sObj["endM"].toInt()   };
            TimeSlot slot(day, ctStart, ctEnd);

            Instructor*   inst = appManager.findInstructorById(sObj["instructor"].toString().toStdString());
            if (!inst) inst = appManager.findInstructorByName(sObj["instructor"].toString().toStdString());
            Course*       crs  = appManager.findCourseByCode(sObj["course"].toString().toStdString());
            Room*         rm   = appManager.findRoomById(sObj["room"].toString().toStdString());
            StudentBatch* btch = appManager.findBatchById(sObj["batch"].toString().toStdString());
            std::string sessionId = sObj.contains("sessionId") ? sObj["sessionId"].toString().toStdString() : "";

            if (inst && crs && rm && btch) {
                ConstraintSettings loadCS;
                loadCS.lunchBreakEnabled = false;
                appManager.validateAndAddClassSession(ClassSession(slot, inst, crs, rm, btch, sessionId), loadCS);
            }
        }
    }

    outLoaded = true;
    if (rootObj.contains("constraints") && rootObj["constraints"].isObject()) {
        return rootObj["constraints"].toObject();
    }
    return emptyObj;
}

} // namespace MainWindowHelper
