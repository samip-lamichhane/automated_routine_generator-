#include "AppManager.hpp"
#include <algorithm>

// ──────────────────────────────────────────────────────────────────────────────
// Basic CRUD
// ──────────────────────────────────────────────────────────────────────────────

void AppManager::addInstructor(const Instructor &instructor) {
  m_masterInstructors.push_back(instructor);
}
void AppManager::addRoom(const Room &room) { m_masterRooms.push_back(room); }
void AppManager::addCourse(const Course &course) {
  m_masterCourses.push_back(course);
}
void AppManager::addBatch(const StudentBatch &batch) {
  m_masterBatches.push_back(batch);
}

// ──────────────────────────────────────────────────────────────────────────────
// Session validation + insertion
// ──────────────────────────────────────────────────────────────────────────────

std::string
AppManager::validateAndAddClassSession(const ClassSession &session,
                                       const ConstraintSettings &cs) {
  TimeSlot newSlot = session.getTimeSlot();

  // Lunch break overlap — use configurable window from ConstraintSettings.
  if (cs.lunchBreakEnabled) {
    // Allow at most 20 minutes of overlap (leaves ≥40 min of break).
    if (newSlot.lunchBreakOverlapMinutes(cs.lunchStartMinutes,
                                         cs.lunchEndMinutes) > 20) {
      return "Cannot schedule: Class eats into the lunch break. "
             "At least 40 minutes of break must remain.";
    }
  }

  // Conflict checks (each guarded by its rule toggle)
  for (const auto &existing : m_timetable) {
    if (!existing.getTimeSlot().overlapsWith(newSlot))
      continue;

    if (cs.ruleNoInstructorDoubleBook &&
        existing.getTeacherId()->getId() == session.getTeacherId()->getId()) {
      return "Teacher Conflict: Instructor is already teaching at this time.";
    }
    if (cs.ruleNoRoomDoubleBook &&
        existing.getRoomId()->getRoomId() == session.getRoomId()->getRoomId()) {
      return "Classroom Conflict: Room is already booked at this time.";
    }
    if (cs.ruleNoBatchClash && existing.getBatchId()->getBatchId() ==
                                   session.getBatchId()->getBatchId()) {
      return "Student Class Conflict: Batch already has a class at this time.";
    }
  }

  m_timetable.push_back(session);
  return "";
}

bool AppManager::removeClassSession(const std::string &sessionId) {
  auto it = std::find_if(m_timetable.begin(), m_timetable.end(),
                         [&sessionId](const ClassSession &s) {
                           return s.getSessionId() == sessionId;
                         });

  if (it != m_timetable.end()) {
    Instructor *teacher = it->getTeacherId();
    Course *course = it->getSubjectId();
    if (teacher && course) {
      // Legacy bookkeeping removed.
    }
    m_timetable.erase(it);
    return true;
  }
  return false;
}

// ──────────────────────────────────────────────────────────────────────────────
// In-place session update (Edit mode)
// ──────────────────────────────────────────────────────────────────────────────

std::string
AppManager::validateAndUpdateClassSession(const std::string &editingSessionId,
                                          const ClassSession &updatedSession,
                                          const ConstraintSettings &cs) {
  TimeSlot newSlot = updatedSession.getTimeSlot();

  // Lunch break overlap check (same rule as Add)
  if (cs.lunchBreakEnabled) {
    if (newSlot.lunchBreakOverlapMinutes(cs.lunchStartMinutes,
                                         cs.lunchEndMinutes) > 20) {
      return "Cannot schedule: Class eats into the lunch break. "
             "At least 40 minutes of break must remain.";
    }
  }

  // Conflict checks — skip the session being edited to avoid self-clash
  for (const auto &existing : m_timetable) {
    if (existing.getSessionId() == editingSessionId)
      continue; // never compare against self

    if (!existing.getTimeSlot().overlapsWith(newSlot))
      continue;

    if (cs.ruleNoInstructorDoubleBook &&
        existing.getTeacherId()->getId() == updatedSession.getTeacherId()->getId()) {
      return "Teacher Conflict: Instructor is already teaching at this time.";
    }
    if (cs.ruleNoRoomDoubleBook &&
        existing.getRoomId()->getRoomId() == updatedSession.getRoomId()->getRoomId()) {
      return "Classroom Conflict: Room is already booked at this time.";
    }
    if (cs.ruleNoBatchClash &&
        existing.getBatchId()->getBatchId() == updatedSession.getBatchId()->getBatchId()) {
      return "Student Class Conflict: Batch already has a class at this time.";
    }
  }

  // Find the existing session and update it in-place
  for (auto &sess : m_timetable) {
    if (sess.getSessionId() != editingSessionId)
      continue;

    // Legacy bookkeeping removed.

    // Replace the session with the updated one (preserves sessionId)
    sess = updatedSession;
    return "";
  }

  return "Session not found: the session to edit no longer exists.";
}


// ──────────────────────────────────────────────────────────────────────────────
// Finders
// ──────────────────────────────────────────────────────────────────────────────

Instructor *AppManager::findInstructorByName(const std::string &name) {
  for (auto &inst : m_masterInstructors)
    if (inst.getName() == name)
      return &inst;
  return nullptr;
}

Instructor *AppManager::findInstructorById(const std::string &id) {
  for (auto &inst : m_masterInstructors)
    if (inst.getId() == id)
      return &inst;
  return nullptr;
}

Course *AppManager::findCourseByCode(const std::string &code) {
  for (auto &crs : m_masterCourses)
    if (crs.getCourseCode() == code)
      return &crs;
  return nullptr;
}

Room *AppManager::findRoomById(const std::string &id) {
  for (auto &rm : m_masterRooms)
    if (rm.getRoomId() == id)
      return &rm;
  return nullptr;
}

StudentBatch *AppManager::findBatchById(const std::string &id) {
  for (auto &b : m_masterBatches)
    if (b.getBatchId() == id)
      return &b;
  return nullptr;
}

// ──────────────────────────────────────────────────────────────────────────────
// Getters
// ──────────────────────────────────────────────────────────────────────────────

const std::vector<ClassSession> &AppManager::getTimetable() const {
  return m_timetable;
}
const std::vector<Instructor> &AppManager::getInstructors() const {
  return m_masterInstructors;
}
const std::vector<Course> &AppManager::getCourses() const {
  return m_masterCourses;
}
const std::vector<Room> &AppManager::getRooms() const { return m_masterRooms; }
const std::vector<StudentBatch> &AppManager::getBatches() const {
  return m_masterBatches;
}

// ──────────────────────────────────────────────────────────────────────────────
// Removal
// ──────────────────────────────────────────────────────────────────────────────

bool AppManager::removeInstructor(const std::string &id) {
  for (auto it = m_masterInstructors.begin(); it != m_masterInstructors.end();
       ++it) {
    if (it->getId() == id) {
      m_masterInstructors.erase(it);
      return true;
    }
  }
  return false;
}

bool AppManager::removeCourse(const std::string &code) {
  for (auto it = m_masterCourses.begin(); it != m_masterCourses.end(); ++it) {
    if (it->getCourseCode() == code) {
      m_masterCourses.erase(it);
      return true;
    }
  }
  return false;
}

bool AppManager::removeRoom(const std::string &id) {
  for (auto it = m_masterRooms.begin(); it != m_masterRooms.end(); ++it) {
    if (it->getRoomId() == id) {
      m_masterRooms.erase(it);
      return true;
    }
  }
  return false;
}

bool AppManager::removeBatch(const std::string &id) {
  for (auto it = m_masterBatches.begin(); it != m_masterBatches.end(); ++it) {
    if (it->getBatchId() == id) {
      m_masterBatches.erase(it);
      return true;
    }
  }
  return false;
}

// ──────────────────────────────────────────────────────────────────────────────
// Usage checks
// ──────────────────────────────────────────────────────────────────────────────

bool AppManager::isInstructorUsed(const std::string &id) const {
  for (const auto &sess : m_timetable)
    if (sess.getTeacherId() && sess.getTeacherId()->getId() == id)
      return true;
  return false;
}

bool AppManager::isCourseUsed(const std::string &code) const {
  for (const auto &sess : m_timetable)
    if (sess.getSubjectId() && sess.getSubjectId()->getCourseCode() == code)
      return true;
  return false;
}

bool AppManager::isRoomUsed(const std::string &id) const {
  for (const auto &sess : m_timetable)
    if (sess.getRoomId() && sess.getRoomId()->getRoomId() == id)
      return true;
  return false;
}

bool AppManager::isBatchUsed(const std::string &id) const {
  for (const auto &sess : m_timetable)
    if (sess.getBatchId() && sess.getBatchId()->getBatchId() == id)
      return true;
  return false;
}

// ──────────────────────────────────────────────────────────────────────────────
// Update helpers
// ──────────────────────────────────────────────────────────────────────────────

bool AppManager::updateInstructor(const Instructor &instructor) {
  for (auto &inst : m_masterInstructors)
    if (inst.getId() == instructor.getId()) {
      inst = instructor;
      return true;
    }
  return false;
}

bool AppManager::updateCourse(const Course &course) {
  for (auto &crs : m_masterCourses)
    if (crs.getCourseCode() == course.getCourseCode()) {
      crs = course;
      return true;
    }
  return false;
}

bool AppManager::updateRoom(const Room &room) {
  for (auto &rm : m_masterRooms)
    if (rm.getRoomId() == room.getRoomId()) {
      rm = room;
      return true;
    }
  return false;
}

bool AppManager::updateBatch(const StudentBatch &batch) {
  for (auto &b : m_masterBatches)
    if (b.getBatchId() == batch.getBatchId()) {
      b = batch;
      return true;
    }
  return false;
}

// ──────────────────────────────────────────────────────────────────────────────
// Busy-check helpers
// ──────────────────────────────────────────────────────────────────────────────

bool AppManager::isInstructorBusyAt(const std::string &id,
                                    const TimeSlot &slot) const {
  for (const auto &sess : m_timetable)
    if (sess.getTeacherId() && sess.getTeacherId()->getId() == id &&
        sess.getTimeSlot().overlapsWith(slot))
      return true;
  return false;
}

bool AppManager::isRoomBusyAt(const std::string &id,
                              const TimeSlot &slot) const {
  for (const auto &sess : m_timetable)
    if (sess.getRoomId() && sess.getRoomId()->getRoomId() == id &&
        sess.getTimeSlot().overlapsWith(slot))
      return true;
  return false;
}

bool AppManager::isBatchBusyAt(const std::string &id,
                               const TimeSlot &slot) const {
  for (const auto &sess : m_timetable)
    if (sess.getBatchId() && sess.getBatchId()->getBatchId() == id &&
        sess.getTimeSlot().overlapsWith(slot))
      return true;
  return false;
}

// ──────────────────────────────────────────────────────────────────────────────
// Auto-generation private helpers
// ──────────────────────────────────────────────────────────────────────────────

// Map Day enum → UI working-day array index
// UI array: [0]=Sunday,[1]=Mon,[2]=Tue,[3]=Wed,[4]=Thu,[5]=Fri,[6]=Sat
static int dayToUIIndex(Day d) {
  switch (d) {
  case Day::Sunday:
    return 0;
  case Day::Monday:
    return 1;
  case Day::Tuesday:
    return 2;
  case Day::Wednesday:
    return 3;
  case Day::Thursday:
    return 4;
  case Day::Friday:
    return 5;
  case Day::Saturday:
    return 6;
  default:
    return -1;
  }
}

bool AppManager::isWorkingDay(Day d, const ConstraintSettings &cs) {
  int idx = dayToUIIndex(d);
  return (idx >= 0) ? cs.workingDays[idx] : false;
}

// Build a list of {startMin, endMin} 1-hour slot pairs for one working day,
// based on the configurable window and optional lunch break.
std::vector<std::pair<int, int>>
AppManager::buildSlotDefs(const ConstraintSettings &cs) {
  std::vector<std::pair<int, int>> slots;
  int cur = cs.dayStartMinutes;
  while (cur + 60 <= cs.dayEndMinutes) {
    int slotEnd = cur + 60;
    // Skip if the slot overlaps with the lunch break
    if (cs.lunchBreakEnabled) {
      // Overlap: slot overlaps lunch if cur < lunchEnd && slotEnd > lunchStart
      bool overlapsLunch =
          (cur < cs.lunchEndMinutes) && (slotEnd > cs.lunchStartMinutes);
      if (!overlapsLunch) {
        slots.push_back({cur, slotEnd});
      }
      // If this slot started before lunch and we are now at the lunch start,
      // jump the cursor past the lunch end so we don't stall in an infinite
      // loop.
      if (cur < cs.lunchStartMinutes && slotEnd > cs.lunchStartMinutes) {
        cur = cs.lunchEndMinutes;
        continue;
      }
    } else {
      slots.push_back({cur, slotEnd});
    }
    cur += 60;
  }
  return slots;
}

// How many minutes the instructor has been teaching *consecutively* on `day`
// immediately before `slotStartMin` (i.e., adjacent sessions with no gap).
int AppManager::consecutiveTeachingMinutes(const std::string &instId, Day day,
                                           int slotStartMin) const {
  // Collect all end-times for this instructor on this day
  std::vector<int> endTimes;
  for (const auto &sess : m_timetable) {
    if (sess.getTeacherId() && sess.getTeacherId()->getId() == instId &&
        sess.getTimeSlot().getDay() == day) {
      endTimes.push_back(sess.getTimeSlot().getEndTime().hours * 60 +
                         sess.getTimeSlot().getEndTime().minutes);
    }
  }

  // Walk backwards from slotStartMin along the chain of contiguous sessions
  int totalMins = 0;
  int cursor = slotStartMin;
  bool found = true;
  while (found) {
    found = false;
    for (int e : endTimes) {
      if (e == cursor) {
        // There is a session ending exactly at cursor — find its start
        for (const auto &sess : m_timetable) {
          if (sess.getTeacherId() && sess.getTeacherId()->getId() == instId &&
              sess.getTimeSlot().getDay() == day) {
            int sEnd = sess.getTimeSlot().getEndTime().hours * 60 +
                       sess.getTimeSlot().getEndTime().minutes;
            int sStart = sess.getTimeSlot().getStartTime().hours * 60 +
                         sess.getTimeSlot().getStartTime().minutes;
            if (sEnd == cursor) {
              totalMins += (sEnd - sStart);
              cursor = sStart;
              found = true;
              break;
            }
          }
        }
        break;
      }
    }
  }
  return totalMins;
}

// Returns true if the course-batch pair already appears on a day that is
// immediately adjacent (previous or next working day) to `day`.
bool AppManager::sameSubjectOnAdjacentDay(
    const std::string &courseCode, const std::string &batchId, Day day,
    const std::vector<Day> &workDays) const {
  // Find the index of `day` in the working-days list
  int dayIdx = -1;
  for (int i = 0; i < static_cast<int>(workDays.size()); ++i) {
    if (workDays[i] == day) {
      dayIdx = i;
      break;
    }
  }
  if (dayIdx < 0)
    return false;

  // Collect adjacent working days (previous and next in the list)
  std::vector<Day> adjacent;
  if (dayIdx > 0)
    adjacent.push_back(workDays[dayIdx - 1]);
  if (dayIdx < static_cast<int>(workDays.size()) - 1)
    adjacent.push_back(workDays[dayIdx + 1]);

  for (const auto &sess : m_timetable) {
    if (sess.getSubjectId() && sess.getBatchId() &&
        sess.getSubjectId()->getCourseCode() == courseCode &&
        sess.getBatchId()->getBatchId() == batchId) {
      for (Day adj : adjacent) {
        if (sess.getTimeSlot().getDay() == adj)
          return true;
      }
    }
  }
  return false;
}

// Returns true if the instructor has a session on a working day adjacent
// to `day` (used for the "Instructor Day Gap" rule).
bool AppManager::instructorOnAdjacentDay(
    const std::string &instId, Day day,
    const std::vector<Day> &workDays) const {
  int dayIdx = -1;
  for (int i = 0; i < static_cast<int>(workDays.size()); ++i) {
    if (workDays[i] == day) {
      dayIdx = i;
      break;
    }
  }
  if (dayIdx < 0)
    return false;

  std::vector<Day> adjacent;
  if (dayIdx > 0)
    adjacent.push_back(workDays[dayIdx - 1]);
  if (dayIdx < static_cast<int>(workDays.size()) - 1)
    adjacent.push_back(workDays[dayIdx + 1]);

  for (const auto &sess : m_timetable) {
    if (sess.getTeacherId() && sess.getTeacherId()->getId() == instId) {
      for (Day adj : adjacent) {
        if (sess.getTimeSlot().getDay() == adj)
          return true;
      }
    }
  }
  return false;
}

// ──────────────────────────────────────────────────────────────────────────────
// clearTimetable
// ──────────────────────────────────────────────────────────────────────────────

void AppManager::clearTimetable() {
  m_timetable.clear();
}

void AppManager::clearAllData() {
  m_masterInstructors.clear();
  m_masterCourses.clear();
  m_masterRooms.clear();
  m_masterBatches.clear();
  m_timetable.clear();
}

int AppManager::countInstructorScheduledHours(const std::string& instructorId, const std::string& skipSessionId) const {
    int count = 0;
    for (const auto& sess : m_timetable) {
        if (!skipSessionId.empty() && sess.getSessionId() == skipSessionId) {
            continue;
        }
        if (sess.getTeacherId() && sess.getTeacherId()->getId() == instructorId) {
            count++;
        }
    }
    return count;
}

// ──────────────────────────────────────────────────────────────────────────────
// autoGenerateTimetable — constraint-driven implementation
// ──────────────────────────────────────────────────────────────────────────────

std::string AppManager::autoGenerateTimetable(const ConstraintSettings &cs) {
  clearTimetable();

  for (auto& inst : m_masterInstructors) {
      inst.resetWeeklyHours();
  }

  // Build the ordered list of working Days
  // We iterate in a natural calendar order: Sun, Mon, Tue, Wed, Thu, Fri, Sat
  const Day calendarOrder[] = {Day::Sunday,    Day::Monday,   Day::Tuesday,
                               Day::Wednesday, Day::Thursday, Day::Friday,
                               Day::Saturday};
  std::vector<Day> workDays;
  // workingDays indices: 0=Sun,1=Mon,2=Tue,3=Wed,4=Thu,5=Fri,6=Sat
  for (int i = 0; i < 7; ++i) {
    if (cs.workingDays[i])
      workDays.push_back(calendarOrder[i]);
  }

  if (workDays.empty())
    return ""; // No working days — nothing to schedule

  // Build slot definitions for one day
  std::vector<std::pair<int, int>> slotDefs = buildSlotDefs(cs);
  if (slotDefs.empty())
    return ""; // No slots fit in the window

  int nextStartDay = 0;

  // For each batch, schedule all courses
  for (size_t bIdx = 0; bIdx < m_masterBatches.size(); ++bIdx) {
    StudentBatch &batch = m_masterBatches[bIdx];

    for (size_t cIdx = 0; cIdx < m_masterCourses.size(); ++cIdx) {
      Course &course = m_masterCourses[cIdx];
      int hoursNeeded = course.getAllocatedHours();
      int hoursScheduled = 0;

      int attempts = 0;
      while (hoursScheduled < hoursNeeded && attempts < 20) {
        ++attempts;
        for (size_t offset = 0;
             offset < workDays.size() && hoursScheduled < hoursNeeded;
             ++offset) {
          size_t dIdx = (nextStartDay + offset) % workDays.size();
          Day day = workDays[dIdx];

          for (size_t sIdx = 0;
               sIdx < slotDefs.size() && hoursScheduled < hoursNeeded; ++sIdx) {
            int startMin = slotDefs[sIdx].first;
            int endMin = slotDefs[sIdx].second;
            ClockTime ctStart{startMin / 60, startMin % 60};
            ClockTime ctEnd{endMin / 60, endMin % 60};
            TimeSlot slot(day, ctStart, ctEnd);

            // ── Batch clash ──────────────────────────────────────
            if (cs.ruleNoBatchClash && isBatchBusyAt(batch.getBatchId(), slot))
              continue;

            // ── No same subject on consecutive days ──────────────
            if (cs.ruleNoSameSubjectConsecDays &&
                sameSubjectOnAdjacentDay(course.getCourseCode(),
                                         batch.getBatchId(), day, workDays))
              continue;

            // ── Find a free, qualified instructor ────────────────
            Instructor *freeInst = nullptr;
            for (size_t iIdx = 0; iIdx < m_masterInstructors.size(); ++iIdx) {
              Instructor &inst = m_masterInstructors[iIdx];

              // Subject-lock qualification
              if (cs.ruleEnforceSubjectLock &&
                  !inst.isQualifiedFor(course.getCourseCode()))
                continue;

              // Instructor double-booking
              if (cs.ruleNoInstructorDoubleBook &&
                  isInstructorBusyAt(inst.getId(), slot))
                continue;

              // Max weekly hours
              if (cs.ruleRespectMaxWeeklyHours) {
                int cur = inst.getHoursUsedThisWeek();
                if (cur + 1 > inst.getMaxLimitHours())
                  continue;
              }

              // Instructor day-gap rule
              if (cs.ruleInstructorDayGap &&
                  instructorOnAdjacentDay(inst.getId(), day, workDays))
                continue;

              // Max consecutive hours per day
              if (cs.ruleMaxConsecHoursEnabled) {
                int consecMins =
                    consecutiveTeachingMinutes(inst.getId(), day, startMin);
                if (consecMins >= cs.ruleMaxConsecHoursPerDay * 60)
                  continue;
              }

              freeInst = &inst;
              break;
            }
            if (!freeInst)
              continue;

            // ── Find a free room ────────────────────────────────
            Room *freeRoom = nullptr;
            for (size_t rIdx = 0; rIdx < m_masterRooms.size(); ++rIdx) {
              Room &rm = m_masterRooms[rIdx];
              if (!cs.ruleNoRoomDoubleBook ||
                  !isRoomBusyAt(rm.getRoomId(), slot)) {
                if (rm.getCapacity() >= batch.getStrength()) {
                  freeRoom = &rm;
                  break;
                }
              }
            }
            if (!freeRoom)
              continue;

            // ── All checks passed — schedule the session ─────────
            freeInst->addWeeklyHours(1);
            m_timetable.push_back(
                ClassSession(slot, freeInst, &course, freeRoom, &batch));
            ++hoursScheduled;

            // One session per day per course-batch to spread things out
            break;
          }
        }
      }

      // Stagger the start day for the next course
      nextStartDay = (nextStartDay + 1) % static_cast<int>(workDays.size());
      
      if (hoursScheduled < hoursNeeded) {
          int remainingCapacity = 0;
          int numQualified = 0;
          for (auto& inst : m_masterInstructors) {
              if (inst.isQualifiedFor(course.getCourseCode())) {
                  remainingCapacity += inst.getRemainingWeeklyHours();
                  numQualified++;
              }
          }
          if (remainingCapacity == 0) {
              return "Course " + course.getCourseCode() + " could not be fully scheduled — all " + std::to_string(numQualified) + " qualified instructors have reached their Max Weekly Hours. Consider adding another instructor qualified for " + course.getCourseCode() + " or increasing existing instructors' Max Weekly Hours.";
          }
      }
    }
  }
  return "";
}