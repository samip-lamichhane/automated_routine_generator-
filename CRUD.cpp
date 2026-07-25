#include "CRUD.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QStringList>

CRUDManager::CRUDManager(MainWindow *mainWindow, QObject *parent)
    : QObject(parent), m_mainWindow(mainWindow)
{
}

void CRUDManager::onAddInstructor()
{
    std::string id   = m_mainWindow->ui->instIdEdit->text().trimmed().toStdString();
    std::string name = m_mainWindow->ui->instNameEdit->text().trimmed().toStdString();
    int maxHours     = m_mainWindow->ui->instHoursSpin->value();

    if (id.empty()) {
        QMessageBox::warning(m_mainWindow, "Validation Error", "Instructor ID cannot be empty.");
        return;
    }
    if (name.empty()) {
        QMessageBox::warning(m_mainWindow, "Validation Error", "Instructor Name cannot be empty.");
        return;
    }

    bool isEditing = !m_mainWindow->m_editingInstId.empty();

    if (!isEditing && m_mainWindow->m_appManager.findInstructorById(id)) {
        QMessageBox::warning(m_mainWindow, "Validation Error", "Instructor ID must be unique.");
        return;
    }

    std::vector<std::string> lockedSubjects;
    if (isEditing && m_mainWindow->m_appManager.isInstructorUsed(m_mainWindow->m_editingInstId)) {
        Instructor* existing = m_mainWindow->m_appManager.findInstructorById(m_mainWindow->m_editingInstId);
        if (existing) lockedSubjects = existing->getLockedSubjects();
    } else {
        for (int i = 0; i < m_mainWindow->m_instSubjectCombos.size(); ++i) {
            QComboBox* cb = m_mainWindow->m_instSubjectCombos[i];
            if (cb->currentIndex() == 0) {
                QMessageBox::warning(m_mainWindow, "Validation Error",
                    QString("Please select a course for Subject %1.").arg(i + 1));
                return;
            }
            std::string code = cb->currentData().toString().toStdString();
            for (const auto& already : lockedSubjects) {
                if (already == code) {
                    QMessageBox::warning(m_mainWindow, "Validation Error",
                        QString("Subject %1 (\"%2\") is already selected.")
                            .arg(i + 1).arg(cb->currentText()));
                    return;
                }
            }
            lockedSubjects.push_back(code);
        }
        if (lockedSubjects.empty()) {
            QMessageBox::warning(m_mainWindow, "Validation Error",
                "At least one subject must be assigned to the instructor.");
            return;
        }
    }

    Instructor inst(id, name, maxHours);
    inst.setLockedSubjects(lockedSubjects);

    if (isEditing) {
        m_mainWindow->m_appManager.updateInstructor(inst);
        m_mainWindow->m_editingInstId = "";
        m_mainWindow->ui->instIdEdit->setEnabled(true);
        m_mainWindow->ui->instSubjectCountSpin->setEnabled(true);
    } else {
        m_mainWindow->m_appManager.addInstructor(inst);
    }

    m_mainWindow->ui->instIdEdit->clear();
    m_mainWindow->ui->instNameEdit->clear();
    m_mainWindow->ui->instHoursSpin->setValue(20);
    m_mainWindow->ui->instSubjectCountSpin->setValue(1);
    m_mainWindow->ui->instSubjectCountSpin->setEnabled(true);
    m_mainWindow->rebuildSubjectCombos(1);

    m_mainWindow->refreshInstList();
    m_mainWindow->saveToFile();
    m_mainWindow->populateCombos();
    m_mainWindow->markConstraintsDirty();
}

void CRUDManager::onEditInstructor()
{
    QListWidgetItem *item = m_mainWindow->ui->instList->currentItem();
    if (!item) {
        QMessageBox::warning(m_mainWindow, "Selection Error", "No instructor selected to edit.");
        return;
    }

    QString text = item->text();
    int parenIdx = text.indexOf(" (Max Hours:");
    QString displayName = (parenIdx != -1) ? text.left(parenIdx) : text;

    Instructor* inst = m_mainWindow->m_appManager.findInstructorByName(displayName.toStdString());
    if (!inst) inst  = m_mainWindow->m_appManager.findInstructorById(displayName.toStdString());
    if (!inst) {
        QMessageBox::warning(m_mainWindow, "Error", "Selected instructor not found.");
        return;
    }

    m_mainWindow->ui->instIdEdit->setText(QString::fromStdString(inst->getId()));
    m_mainWindow->ui->instNameEdit->setText(QString::fromStdString(inst->getName()));
    m_mainWindow->ui->instHoursSpin->setValue(inst->getMaxLimitHours());

    m_mainWindow->m_editingInstId = inst->getId();
    m_mainWindow->ui->instIdEdit->setEnabled(false);

    const auto& locked = inst->getLockedSubjects();
    bool isUsed = m_mainWindow->m_appManager.isInstructorUsed(inst->getId());

    if (isUsed) {
        m_mainWindow->ui->instSubjectCountSpin->setValue(static_cast<int>(locked.size() > 0 ? locked.size() : 1));
        m_mainWindow->ui->instSubjectCountSpin->setEnabled(false);
        m_mainWindow->rebuildSubjectCombos(static_cast<int>(locked.size() > 0 ? locked.size() : 1));
        for (int i = 0; i < m_mainWindow->m_instSubjectCombos.size() && i < static_cast<int>(locked.size()); ++i) {
            int idx = m_mainWindow->m_instSubjectCombos[i]->findData(QString::fromStdString(locked[i]));
            if (idx != -1) m_mainWindow->m_instSubjectCombos[i]->setCurrentIndex(idx);
            m_mainWindow->m_instSubjectCombos[i]->setEnabled(false);
        }
        QMessageBox::information(m_mainWindow, "Subject List Locked",
            "This instructor has scheduled sessions. Name and Max Hours can still be edited,\n"
            "but the subject list is locked and cannot be changed.");
    } else {
        int subCount = static_cast<int>(locked.size() > 0 ? locked.size() : 1);
        m_mainWindow->ui->instSubjectCountSpin->setValue(subCount);
        m_mainWindow->ui->instSubjectCountSpin->setEnabled(true);
        m_mainWindow->rebuildSubjectCombos(subCount);
        for (int i = 0; i < m_mainWindow->m_instSubjectCombos.size() && i < static_cast<int>(locked.size()); ++i) {
            int idx = m_mainWindow->m_instSubjectCombos[i]->findData(QString::fromStdString(locked[i]));
            if (idx != -1) m_mainWindow->m_instSubjectCombos[i]->setCurrentIndex(idx);
        }
    }
}

void CRUDManager::onDeleteInstructor()
{
    QListWidgetItem *item = m_mainWindow->ui->instList->currentItem();
    if (!item) {
        QMessageBox::warning(m_mainWindow, "Selection Error", "No instructor selected to delete.");
        return;
    }

    QString text = item->text();
    int parenIdx = text.indexOf(" (Max Hours:");
    QString displayName = (parenIdx != -1) ? text.left(parenIdx) : text;

    Instructor* inst = m_mainWindow->m_appManager.findInstructorByName(displayName.toStdString());
    if (!inst) inst  = m_mainWindow->m_appManager.findInstructorById(displayName.toStdString());
    if (!inst) {
        QMessageBox::warning(m_mainWindow, "Error", "Selected instructor not found.");
        return;
    }

    if (m_mainWindow->m_appManager.isInstructorUsed(inst->getId())) {
        QMessageBox::critical(m_mainWindow, "Cannot Delete",
            "This instructor is used in scheduled sessions and cannot be deleted.");
        return;
    }

    if (QMessageBox::question(m_mainWindow, "Confirm Delete",
            "Are you sure you want to delete this instructor?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    if (m_mainWindow->m_appManager.removeInstructor(inst->getId())) {
        delete item;
        m_mainWindow->saveToFile();
        m_mainWindow->populateCombos();
        m_mainWindow->markConstraintsDirty();
    } else {
        QMessageBox::warning(m_mainWindow, "Delete Failed", "Unable to delete instructor.");
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// COURSE CRUD
// ──────────────────────────────────────────────────────────────────────────────

void CRUDManager::onAddCourse()
{
    std::string code = m_mainWindow->ui->courseCodeEdit->text().trimmed().toStdString();
    std::string name = m_mainWindow->ui->courseNameEdit->text().trimmed().toStdString();
    int hours        = m_mainWindow->ui->courseHoursSpin->value();

    if (code.empty()) {
        QMessageBox::warning(m_mainWindow, "Validation Error", "Course Code cannot be empty.");
        return;
    }
    if (m_mainWindow->m_editingCourseCode.empty() || m_mainWindow->m_editingCourseCode != code) {
        if (m_mainWindow->m_appManager.findCourseByCode(code)) {
            QMessageBox::warning(m_mainWindow, "Validation Error", "Course Code must be unique.");
            return;
        }
    }
    if (name.empty()) {
        QMessageBox::warning(m_mainWindow, "Validation Error", "Course Name cannot be empty.");
        return;
    }

    Course crs(code, name, hours);

    if (!m_mainWindow->m_editingCourseCode.empty()) {
        m_mainWindow->m_appManager.updateCourse(crs);
        m_mainWindow->m_editingCourseCode = "";
        m_mainWindow->ui->courseCodeEdit->setEnabled(true);
    } else {
        m_mainWindow->m_appManager.addCourse(crs);
    }

    m_mainWindow->ui->courseCodeEdit->clear();
    m_mainWindow->ui->courseNameEdit->clear();
    m_mainWindow->ui->courseHoursSpin->setValue(3);

    m_mainWindow->ui->courseList->clear();
    for (const auto& c : m_mainWindow->m_appManager.getCourses()) {
        m_mainWindow->ui->courseList->addItem(QString("%1 (Allocated Hours: %2)")
            .arg(QString::fromStdString(c.getCourseCode()))
            .arg(c.getAllocatedHours()));
    }

    m_mainWindow->saveToFile();
    m_mainWindow->populateCombos();
    m_mainWindow->markConstraintsDirty();
}

void CRUDManager::onEditCourse()
{
    QListWidgetItem *item = m_mainWindow->ui->courseList->currentItem();
    if (!item) {
        QMessageBox::warning(m_mainWindow, "Selection Error", "No course selected to edit.");
        return;
    }

    QString text = item->text();
    int idx = text.indexOf(" (Allocated Hours:");
    std::string code = (idx != -1) ? text.left(idx).toStdString() : text.toStdString();

    Course* crs = m_mainWindow->m_appManager.findCourseByCode(code);
    if (!crs) {
        QMessageBox::warning(m_mainWindow, "Error", "Selected course not found.");
        return;
    }

    m_mainWindow->ui->courseCodeEdit->setText(QString::fromStdString(crs->getCourseCode()));
    m_mainWindow->ui->courseNameEdit->setText(QString::fromStdString(crs->getName()));
    m_mainWindow->ui->courseHoursSpin->setValue(crs->getAllocatedHours());

    m_mainWindow->m_editingCourseCode = crs->getCourseCode();
    m_mainWindow->ui->courseCodeEdit->setEnabled(false);
}

void CRUDManager::onDeleteCourse()
{
    QListWidgetItem *item = m_mainWindow->ui->courseList->currentItem();
    if (!item) {
        QMessageBox::warning(m_mainWindow, "Selection Error", "No course selected to delete.");
        return;
    }

    QString text = item->text();
    int idx = text.indexOf(" (Allocated Hours:");
    std::string code = (idx != -1) ? text.left(idx).toStdString() : text.toStdString();

    if (m_mainWindow->m_appManager.isCourseUsed(code)) {
        QMessageBox::critical(m_mainWindow, "Cannot Delete",
            "This course is used in scheduled sessions and cannot be deleted.");
        return;
    }

    if (QMessageBox::question(m_mainWindow, "Confirm Delete", "Are you sure you want to delete this course?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    if (m_mainWindow->m_appManager.removeCourse(code)) {
        delete item;
        m_mainWindow->saveToFile();
        m_mainWindow->populateCombos();
        m_mainWindow->markConstraintsDirty();
    } else {
        QMessageBox::warning(m_mainWindow, "Delete Failed", "Unable to delete course.");
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// ROOM CRUD
// ──────────────────────────────────────────────────────────────────────────────

void CRUDManager::onAddRoom()
{
    std::string number   = m_mainWindow->ui->roomIdEdit->text().trimmed().toStdString();
    std::string building = m_mainWindow->ui->roomBuildingEdit->text().trimmed().toStdString();
    int capacity         = m_mainWindow->ui->roomCapSpin->value();
    RoomType type        = static_cast<RoomType>(m_mainWindow->ui->roomTypeCombo->currentIndex());

    if (number.empty()) {
        QMessageBox::warning(m_mainWindow, "Validation Error", "Room Number cannot be empty.");
        return;
    }
    if (m_mainWindow->m_editingRoomId.empty() || m_mainWindow->m_editingRoomId != number) {
        if (m_mainWindow->m_appManager.findRoomById(number)) {
            QMessageBox::warning(m_mainWindow, "Validation Error", "Room Number must be unique.");
            return;
        }
    }
    if (building.empty()) {
        QMessageBox::warning(m_mainWindow, "Validation Error", "Building cannot be empty.");
        return;
    }

    Room rm(number, capacity, type, building);

    if (!m_mainWindow->m_editingRoomId.empty()) {
        m_mainWindow->m_appManager.updateRoom(rm);
        m_mainWindow->m_editingRoomId = "";
        m_mainWindow->ui->roomIdEdit->setEnabled(true);
    } else {
        m_mainWindow->m_appManager.addRoom(rm);
    }

    m_mainWindow->ui->roomIdEdit->clear();
    m_mainWindow->ui->roomBuildingEdit->clear();
    m_mainWindow->ui->roomCapSpin->setValue(60);
    m_mainWindow->ui->roomTypeCombo->setCurrentIndex(0);

    m_mainWindow->ui->roomList->clear();
    for (const auto& r : m_mainWindow->m_appManager.getRooms()) {
        m_mainWindow->ui->roomList->addItem(QString("%1 (Capacity: %2, Type: %3)")
            .arg(QString::fromStdString(r.getRoomId()))
            .arg(r.getCapacity())
            .arg(QString::fromStdString(r.getTypeAsString())));
    }

    m_mainWindow->saveToFile();
    m_mainWindow->populateCombos();
    m_mainWindow->markConstraintsDirty();
}

void CRUDManager::onEditRoom()
{
    QListWidgetItem *item = m_mainWindow->ui->roomList->currentItem();
    if (!item) {
        QMessageBox::warning(m_mainWindow, "Selection Error", "No room selected to edit.");
        return;
    }

    QString text = item->text();
    int idx = text.indexOf(" (Capacity:");
    std::string number = (idx != -1) ? text.left(idx).toStdString() : text.toStdString();

    Room* rm = m_mainWindow->m_appManager.findRoomById(number);
    if (!rm) {
        QMessageBox::warning(m_mainWindow, "Error", "Selected room not found.");
        return;
    }

    m_mainWindow->ui->roomIdEdit->setText(QString::fromStdString(rm->getRoomId()));
    m_mainWindow->ui->roomBuildingEdit->setText(QString::fromStdString(rm->getBuilding()));
    m_mainWindow->ui->roomCapSpin->setValue(rm->getCapacity());
    m_mainWindow->ui->roomTypeCombo->setCurrentIndex(static_cast<int>(rm->getType()));

    m_mainWindow->m_editingRoomId = rm->getRoomId();
    m_mainWindow->ui->roomIdEdit->setEnabled(false);
}

void CRUDManager::onDeleteRoom()
{
    QListWidgetItem *item = m_mainWindow->ui->roomList->currentItem();
    if (!item) {
        QMessageBox::warning(m_mainWindow, "Selection Error", "No room selected to delete.");
        return;
    }

    QString text = item->text();
    int idx = text.indexOf(" (Capacity:");
    std::string number = (idx != -1) ? text.left(idx).toStdString() : text.toStdString();

    if (m_mainWindow->m_appManager.isRoomUsed(number)) {
        QMessageBox::critical(m_mainWindow, "Cannot Delete",
            "This room is used in scheduled sessions and cannot be deleted.");
        return;
    }

    if (QMessageBox::question(m_mainWindow, "Confirm Delete", "Are you sure you want to delete this room?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    if (m_mainWindow->m_appManager.removeRoom(number)) {
        delete item;
        m_mainWindow->saveToFile();
        m_mainWindow->populateCombos();
        m_mainWindow->markConstraintsDirty();
    } else {
        QMessageBox::warning(m_mainWindow, "Delete Failed", "Unable to delete room.");
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// BATCH CRUD
// ──────────────────────────────────────────────────────────────────────────────

void CRUDManager::onAddBatch()
{
    std::string name = m_mainWindow->ui->batchIdEdit->text().trimmed().toStdString();
    std::string dept = m_mainWindow->ui->batchDeptEdit->text().trimmed().toStdString();
    int strength     = m_mainWindow->ui->batchStrengthSpin->value();
    ProgramType prog = static_cast<ProgramType>(m_mainWindow->ui->batchProgCombo->currentIndex());

    if (name.empty()) {
        QMessageBox::warning(m_mainWindow, "Validation Error", "Batch Name cannot be empty.");
        return;
    }
    if (m_mainWindow->m_editingBatchId.empty() || m_mainWindow->m_editingBatchId != name) {
        if (m_mainWindow->m_appManager.findBatchById(name)) {
            QMessageBox::warning(m_mainWindow, "Validation Error", "Batch Name must be unique.");
            return;
        }
    }
    if (dept.empty()) {
        QMessageBox::warning(m_mainWindow, "Validation Error", "Department cannot be empty.");
        return;
    }

    StudentBatch b(name, strength, prog, dept);

    if (!m_mainWindow->m_editingBatchId.empty()) {
        m_mainWindow->m_appManager.updateBatch(b);
        m_mainWindow->m_editingBatchId = "";
        m_mainWindow->ui->batchIdEdit->setEnabled(true);
    } else {
        m_mainWindow->m_appManager.addBatch(b);
    }

    m_mainWindow->ui->batchIdEdit->clear();
    m_mainWindow->ui->batchDeptEdit->clear();
    m_mainWindow->ui->batchStrengthSpin->setValue(45);
    m_mainWindow->ui->batchProgCombo->setCurrentIndex(0);

    m_mainWindow->ui->batchList->clear();
    for (const auto& bat : m_mainWindow->m_appManager.getBatches()) {
        m_mainWindow->ui->batchList->addItem(QString("%1 (Strength: %2, Program: %3)")
            .arg(QString::fromStdString(bat.getBatchId()))
            .arg(bat.getStrength())
            .arg(QString::fromStdString(bat.getProgramAsString())));
    }

    m_mainWindow->saveToFile();
    m_mainWindow->populateCombos();
    m_mainWindow->markConstraintsDirty();
}

void CRUDManager::onEditBatch()
{
    QListWidgetItem *item = m_mainWindow->ui->batchList->currentItem();
    if (!item) {
        QMessageBox::warning(m_mainWindow, "Selection Error", "No student batch selected to edit.");
        return;
    }

    QString text = item->text();
    int idx = text.indexOf(" (Strength:");
    std::string name = (idx != -1) ? text.left(idx).toStdString() : text.toStdString();

    StudentBatch* b = m_mainWindow->m_appManager.findBatchById(name);
    if (!b) {
        QMessageBox::warning(m_mainWindow, "Error", "Selected student batch not found.");
        return;
    }

    m_mainWindow->ui->batchIdEdit->setText(QString::fromStdString(b->getBatchId()));
    m_mainWindow->ui->batchDeptEdit->setText(QString::fromStdString(b->getDepartment()));
    m_mainWindow->ui->batchStrengthSpin->setValue(b->getStrength());
    m_mainWindow->ui->batchProgCombo->setCurrentIndex(static_cast<int>(b->getProgram()));

    m_mainWindow->m_editingBatchId = b->getBatchId();
    m_mainWindow->ui->batchIdEdit->setEnabled(false);
}

void CRUDManager::onDeleteBatch()
{
    QListWidgetItem *item = m_mainWindow->ui->batchList->currentItem();
    if (!item) {
        QMessageBox::warning(m_mainWindow, "Selection Error", "No student batch selected to delete.");
        return;
    }

    QString text = item->text();
    int idx = text.indexOf(" (Strength:");
    std::string name = (idx != -1) ? text.left(idx).toStdString() : text.toStdString();

    if (m_mainWindow->m_appManager.isBatchUsed(name)) {
        QMessageBox::critical(m_mainWindow, "Cannot Delete",
            "This batch is used in scheduled sessions and cannot be deleted.");
        return;
    }

    if (QMessageBox::question(m_mainWindow, "Confirm Delete",
            "Are you sure you want to delete this student batch?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    if (m_mainWindow->m_appManager.removeBatch(name)) {
        delete item;
        m_mainWindow->saveToFile();
        m_mainWindow->populateCombos();
        m_mainWindow->markConstraintsDirty();
    } else {
        QMessageBox::warning(m_mainWindow, "Delete Failed", "Unable to delete student batch.");
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// SESSION CRUD
// ──────────────────────────────────────────────────────────────────────────────

void CRUDManager::onAddClassSession()
{
    bool isEditMode = !m_mainWindow->m_editingSessionId.isEmpty();

    QString instName   = m_mainWindow->m_sessInstCombo->currentText();
    QString courseCode = m_mainWindow->m_sessCourseCombo->currentData().toString();
    QString roomId     = m_mainWindow->m_sessRoomCombo->currentData().toString();
    QString batchId    = m_mainWindow->m_sessBatchCombo->currentData().toString();

    if (instName.isEmpty() || courseCode.isEmpty() || roomId.isEmpty() || batchId.isEmpty()) {
        QMessageBox::warning(m_mainWindow, "Selection Error",
            "Please ensure all entities (Instructors, Courses, Rooms, Batches) "
            "are created and selected.");
        return;
    }

    Instructor*   inst = m_mainWindow->m_appManager.findInstructorByName(instName.toStdString());
    if (!inst)    inst = m_mainWindow->m_appManager.findInstructorById(instName.toStdString());
    Course*       crs  = m_mainWindow->m_appManager.findCourseByCode(courseCode.toStdString());
    Room*         rm   = m_mainWindow->m_appManager.findRoomById(roomId.toStdString());
    StudentBatch* btch = m_mainWindow->m_appManager.findBatchById(batchId.toStdString());

    if (!inst || !crs || !rm || !btch) {
        QMessageBox::critical(m_mainWindow, "System Error",
            "Failed to retrieve references for the selected objects.");
        return;
    }

    // Subject qualification guard (applies in both Add and Edit modes)
    if (!inst->isQualifiedFor(crs->getCourseCode())) {
        QStringList lockedList;
        for (const auto& s : inst->getLockedSubjects())
            lockedList << QString::fromStdString(s);
        QMessageBox::warning(m_mainWindow, "Subject Not Assigned",
            QString("Instructor \"%1\" is not qualified to teach \"%2\".\n\n"
                    "Assigned subjects: %3")
                .arg(QString::fromStdString(inst->getName()))
                .arg(QString::fromStdString(crs->getCourseCode()))
                .arg(lockedList.isEmpty() ? "(none)" : lockedList.join(", ")));
        return;
    }

    QTime startTime = m_mainWindow->m_sessStartEdit->time();
    QTime endTime   = m_mainWindow->m_sessEndEdit->time();

    if (startTime >= endTime) {
        QMessageBox::warning(m_mainWindow, "Time Range Error",
            "End time must be strictly after start time.");
        return;
    }

    // Read current constraints for the time window check
    ConstraintSettings cs = m_mainWindow->readConstraintsFromUI();
    QTime csStart(cs.dayStartMinutes / 60, cs.dayStartMinutes % 60);
    QTime csEnd(cs.dayEndMinutes / 60, cs.dayEndMinutes % 60);

    if (startTime < csStart || endTime > csEnd) {
        QMessageBox::warning(m_mainWindow, "Time Range Error",
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
    Day day = dayMap[m_mainWindow->m_sessDayCombo->currentIndex()];

    ClockTime ctStart{ startTime.hour(), startTime.minute() };
    ClockTime ctEnd{   endTime.hour(),   endTime.minute()   };
    TimeSlot  slot(day, ctStart, ctEnd);

    // Soft constraint: 1-hr break after every 2 back-to-back classes for the batch.
    // In Edit mode, exclude the session being edited from the existing-slots list
    // (it will be replaced, so its old slot shouldn't count against the new one).
    std::vector<TimeSlot> batchDaySlots;
    batchDaySlots.push_back(slot);
    for (const auto& existing : m_mainWindow->m_appManager.getTimetable()) {
        if (isEditMode && existing.getSessionId() == m_mainWindow->m_editingSessionId.toStdString())
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
        if (QMessageBox::question(m_mainWindow, "Batch Overload Warning",
                "This schedule places 3 or more classes for the student batch without a 1-hour break.\n\n"
                "Proceed anyway?",
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;
    }

    if (isEditMode) {
        // ── Edit mode ──────────────────────────────────────────────────────────
        // Find the old session data for the workload check
        std::string oldInstId, oldCrsCode;
        for (const auto& sess : m_mainWindow->m_appManager.getTimetable()) {
            if (sess.getSessionId() == m_mainWindow->m_editingSessionId.toStdString()) {
                oldInstId  = sess.getTeacherId()->getId();
                oldCrsCode = sess.getSubjectId()->getCourseCode();
                break;
            }
        }

        bool instChanged = (oldInstId  != inst->getId());
        bool crsChanged  = (oldCrsCode != crs->getCourseCode());

        // Prospective workload check: only needed when instructor or course changes.
        if (instChanged || crsChanged) {
            int currentHours = m_mainWindow->m_appManager.countInstructorScheduledHours(inst->getId(), m_mainWindow->m_editingSessionId.toStdString());

            if (currentHours + 1 > inst->getMaxLimitHours()) {
                QMessageBox::warning(m_mainWindow, "Workload Limit Exceeded",
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
                                    m_mainWindow->m_editingSessionId.toStdString());

        std::string err = m_mainWindow->m_appManager.validateAndUpdateClassSession(
            m_mainWindow->m_editingSessionId.toStdString(), updatedSession, cs);

        if (!err.empty()) {
            QMessageBox::warning(m_mainWindow, "Scheduling Constraint Violation",
                QString::fromStdString(err));
            return;
        }

        m_mainWindow->saveToFile();
        m_mainWindow->refreshListsAndTables();
        QMessageBox::information(m_mainWindow, "Save Succeeded",
            "Class session updated successfully!");

        if (m_mainWindow->m_addSessionDialog) {
            m_mainWindow->m_addSessionDialog->accept();
        }

    } else {
        // ── Add mode (original behavior, unchanged) ────────────────────────────
        int currentHours = m_mainWindow->m_appManager.countInstructorScheduledHours(inst->getId());
        if (currentHours + 1 > inst->getMaxLimitHours()) {
            QMessageBox::warning(m_mainWindow, "Workload Limit Exceeded",
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
        std::string err = m_mainWindow->m_appManager.validateAndAddClassSession(session, cs);
        if (!err.empty()) {
            QMessageBox::warning(m_mainWindow, "Scheduling Constraint Violation",
                QString::fromStdString(err));
            return;
        }

        m_mainWindow->saveToFile();
        m_mainWindow->refreshListsAndTables();
        QMessageBox::information(m_mainWindow, "Schedule Succeeded",
            "Class session scheduled successfully!");

        if (m_mainWindow->m_addSessionDialog) {
            m_mainWindow->m_addSessionDialog->accept();
        }
    }
}


void CRUDManager::onDeleteClassSession()
{
    std::string sessionId;

    if (m_mainWindow->ui->timetableSubTabs->currentIndex() == 0) {
        // We are on the Schedule (Table) view
        int row = m_mainWindow->ui->timetableTable->currentRow();
        if (row < 0) {
            QMessageBox::warning(m_mainWindow, "Selection Error",
                "No class session selected in the Schedule table to delete.");
            return;
        }

        QTableWidgetItem* item = m_mainWindow->ui->timetableTable->item(row, 0);
        if (!item) return;
        sessionId = item->data(Qt::UserRole).toString().toStdString();
    } else {
        // We are on the Grid View
        int row = m_mainWindow->ui->timetableGrid->currentRow();
        int col = m_mainWindow->ui->timetableGrid->currentColumn();
        if (row < 0 || col < 0) {
            QMessageBox::warning(m_mainWindow, "Selection Error",
                "No class session selected in the Grid View to delete.");
            return;
        }

        QTableWidgetItem* item = m_mainWindow->ui->timetableGrid->item(row, col);
        if (!item || item->data(Qt::UserRole).isNull() || item->data(Qt::UserRole).toString().isEmpty()) {
            QMessageBox::warning(m_mainWindow, "Selection Error",
                "Please select a valid scheduled session block in the Grid View.");
            return;
        }
        sessionId = item->data(Qt::UserRole).toString().toStdString();
    }

    if (QMessageBox::question(m_mainWindow, "Confirm Delete",
            "Are you sure you want to remove this scheduled session?",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) return;

    if (m_mainWindow->m_appManager.removeClassSession(sessionId)) {
        m_mainWindow->saveToFile();
        m_mainWindow->refreshListsAndTables();
    } else {
        QMessageBox::warning(m_mainWindow, "Delete Failed",
            "Unable to remove the selected session.");
    }
}

// ──────────────────────────────────────────────────────────────────────────────
// Reset All Data
// ──────────────────────────────────────────────────────────────────────────────

void CRUDManager::onResetAllData() {
    QMessageBox msgBox(m_mainWindow);
    msgBox.setWindowTitle("Confirm Reset All Data");
    msgBox.setText("This will permanently delete ALL instructors, courses, rooms, student batches, and scheduled sessions. This cannot be undone. Are you sure you want to continue?");
    msgBox.setIcon(QMessageBox::Warning);

    QPushButton *cancelBtn = msgBox.addButton("Cancel", QMessageBox::RejectRole);
    QPushButton *deleteBtn = msgBox.addButton("Delete Everything", QMessageBox::DestructiveRole);
    msgBox.setDefaultButton(cancelBtn);

    msgBox.exec();

    if (msgBox.clickedButton() == deleteBtn) {
        m_mainWindow->m_appManager.clearAllData();

        // Reset Constraints state but NOT the rules
        m_mainWindow->markConstraintsDirty();
        if (m_mainWindow->ui->validationOutput) {
            m_mainWindow->ui->validationOutput->setHtml("");
            m_mainWindow->ui->validationOutput->setPlaceholderText("Click \"Validate Constraints\" to run a feasibility check...");
        }

        // Save empty state to file
        m_mainWindow->saveToFile();

        // Refresh all UI tabs to clear old data from screen
        m_mainWindow->refreshListsAndTables();
        m_mainWindow->refreshInstList();
        m_mainWindow->refreshTimetableGrid();
    }
}
