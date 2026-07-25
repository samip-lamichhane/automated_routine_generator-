#include "schedule_validator.hpp"
#include "AppManager.hpp"
#include <QString>
#include <QStringList>

bool ScheduleValidator::hasInstructorClash(
    const std::vector<ClassSession> &allSessions,
    std::string &outErrorMessage) {
  for (size_t i = 0; i < allSessions.size(); ++i) {
    for (size_t j = i + 1; j < allSessions.size(); ++j) {

      // Extract the instructor pointers from the ClassSession objects
      const Instructor *teacher1 = allSessions[i].getTeacherId();
      const Instructor *teacher2 = allSessions[j].getTeacherId();

      // Safety guard: skip if either session doesn't have an instructor
      // assigned yet
      if (teacher1 == nullptr || teacher2 == nullptr) {
        continue;
      }

      // Compare the memory addresses (pointers) directly
      if (teacher1 == teacher2) {
        // Check if they occur on the same day
        if (allSessions[i].getTimeSlot().getDay() ==
            allSessions[j].getTimeSlot().getDay()) {
          // Check if their execution intervals overlap
          if (allSessions[i].getTimeSlot().overlapsWith(
                  allSessions[j].getTimeSlot())) {

            const Course *subject1 = allSessions[i].getSubjectId();
            const Course *subject2 = allSessions[j].getSubjectId();

            std::string course1 = (subject1 != nullptr)
                                      ? subject1->getCourseCode()
                                      : "Unknown Course";
            std::string course2 = (subject2 != nullptr)
                                      ? subject2->getCourseCode()
                                      : "Unknown Course";

            // Formulate the error message for the Qt UI pop-up window
            outErrorMessage = "Instructor Clash! " + teacher1->getName() +
                              " is double-booked for " + course1 + " and " +
                              course2 + ".";
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool ScheduleValidator::hasRoomClash(
    const std::vector<ClassSession> &allSessions,
    std::string &outErrorMessage) {
  for (size_t i = 0; i < allSessions.size(); ++i) {
    for (size_t j = i + 1; j < allSessions.size(); ++j) {

      // Extract the Room pointers
      const Room *room1 = allSessions[i].getRoomId();
      const Room *room2 = allSessions[j].getRoomId();

      // Safety guard: skip if either session doesn't have a room assigned yet
      if (room1 == nullptr || room2 == nullptr) {
        continue;
      }

      // Compare the Room pointers directly
      if (room1 == room2) {
        if (allSessions[i].getTimeSlot().getDay() ==
            allSessions[j].getTimeSlot().getDay()) {
          if (allSessions[i].getTimeSlot().overlapsWith(
                  allSessions[j].getTimeSlot())) {

            const StudentBatch *batch1 = allSessions[i].getBatchId();
            const StudentBatch *batch2 = allSessions[j].getBatchId();

            std::string b1 =
                (batch1 != nullptr) ? batch1->getBatchId() : "Unknown Batch";
            std::string b2 =
                (batch2 != nullptr) ? batch2->getBatchId() : "Unknown Batch";

            outErrorMessage = "Room Clash! Room " + room1->getRoomId() +
                              " is occupied by both " + b1 + " and " + b2 + ".";
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool ScheduleValidator::hasBatchClash(
    const std::vector<ClassSession> &allSessions,
    std::string &outErrorMessage) {
  for (size_t i = 0; i < allSessions.size(); ++i) {
    for (size_t j = i + 1; j < allSessions.size(); ++j) {

      // Extract StudentBatch pointers
      const StudentBatch *batch1 = allSessions[i].getBatchId();
      const StudentBatch *batch2 = allSessions[j].getBatchId();

      // Safety guard: skip if either session doesn't have a batch assigned yet
      if (batch1 == nullptr || batch2 == nullptr) {
        continue;
      }

      // Compare the StudentBatch pointers directly
      if (batch1 == batch2) {
        if (allSessions[i].getTimeSlot().getDay() ==
            allSessions[j].getTimeSlot().getDay()) {
          if (allSessions[i].getTimeSlot().overlapsWith(
                  allSessions[j].getTimeSlot())) {

            const Course *subject1 = allSessions[i].getSubjectId();
            const Course *subject2 = allSessions[j].getSubjectId();

            std::string course1 = (subject1 != nullptr)
                                      ? subject1->getCourseCode()
                                      : "Unknown Course";
            std::string course2 = (subject2 != nullptr)
                                      ? subject2->getCourseCode()
                                      : "Unknown Course";

            outErrorMessage = "Batch Clash! Student Batch " +
                              batch1->getBatchId() +
                              " has overlapping sessions for " + course1 +
                              " and " + course2 + ".";
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool ScheduleValidator::validateFeasibility(const AppManager& appManager, const ConstraintSettings& cs, std::string& outHtmlLog) {
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

    const auto& courses     = appManager.getCourses();
    const auto& instructors = appManager.getInstructors();
    const auto& rooms       = appManager.getRooms();
    const auto& batches     = appManager.getBatches();

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
    int labCount = 0;
    int theoryCount = 0;
    int audCount = 0;
    for (const auto& r : rooms) {
        if (r.getType() == RoomType::Lab) labCount++;
        else if (r.getType() == RoomType::Theory) theoryCount++;
        else if (r.getType() == RoomType::Auditorium) audCount++;
    }

    if (roomCount >= batchCount) {
        pass(QString("Room count (%1) is sufficient for the number of simultaneous batches (%2).<br>"
                     "&nbsp;&nbsp;&nbsp;&nbsp;Breakdown: %3 Theory, %4 Lab, %5 Auditorium.")
             .arg(roomCount).arg(batchCount).arg(theoryCount).arg(labCount).arg(audCount));
    } else {
        fail(QString("Only %1 room(s) but %2 concurrent batch(es) may need scheduling simultaneously.")
             .arg(roomCount).arg(batchCount));
    }

    outHtmlLog = output.toStdString();
    return allPassed;
}

bool ScheduleValidator::isDataCorrupted(const AppManager& appManager) {
    for (const auto& session : appManager.getTimetable()) {
        std::string genCode = session.getSubjectId()->getCourseCode();
        if (!const_cast<AppManager&>(appManager).findCourseByCode(genCode)) {
            return true;
        }
    }
    return false;
}