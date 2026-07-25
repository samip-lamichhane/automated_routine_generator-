#ifndef APP_MANAGER_HPP
#define APP_MANAGER_HPP

#include <vector>
#include <string>
#include "ConstraintSettings.hpp"
#include "Instructor.hpp"
#include "Course.hpp"
#include "Room.hpp"
#include "Student_Batch.hpp"
#include "classSession.hpp"

class AppManager {
private:
    std::vector<Instructor>    m_masterInstructors;
    std::vector<Course>        m_masterCourses;
    std::vector<Room>          m_masterRooms;
    std::vector<StudentBatch>  m_masterBatches;

    std::vector<ClassSession>  m_timetable;   // final generated container

    // ── Auto-generation helpers ──────────────────────────────────────────────

    // Returns true if the given Day enum value is enabled in ConstraintSettings.
    // workingDays array: [0]=Sunday, [1]=Mon, [2]=Tue, [3]=Wed, [4]=Thu,
    //                    [5]=Fri,    [6]=Saturday
    static bool isWorkingDay(Day d, const ConstraintSettings& cs);

    // Compute the number of 1-hour teaching slots per working day given the
    // configurable time window and optional lunch break.
    static std::vector<std::pair<int,int>> buildSlotDefs(const ConstraintSettings& cs);

    // Returns how many minutes the instructor has been consecutively
    // teaching on the given day up to (but not including) slotStartMin.
    int consecutiveTeachingMinutes(const std::string& instId, Day day,
                                   int slotStartMin) const;

    // Returns true if the same course-batch pair already appears on the
    // day immediately before OR after the given day within the active
    // working-day list.
    bool sameSubjectOnAdjacentDay(const std::string& courseCode,
                                  const std::string& batchId,
                                  Day day,
                                  const std::vector<Day>& workDays) const;

    // Returns true if the instructor has taught on a working day that is
    // immediately adjacent (previous or next) to the given day.
    bool instructorOnAdjacentDay(const std::string& instId,
                                 Day day,
                                 const std::vector<Day>& workDays) const;

public:
    AppManager() = default;

    void addInstructor(const Instructor& instructor);
    void addRoom(const Room& room);
    void addCourse(const Course& course);
    void addBatch(const StudentBatch& batch);

    // Returns an empty string on success, or an error message on failure.
    // Respects the lunch window stored in cs (if provided) for the overlap check.
    std::string validateAndAddClassSession(const ClassSession& session,
                                           const ConstraintSettings& cs = ConstraintSettings{});

    // Validates & updates an existing session in-place (keeps its sessionId).
    // The session identified by editingSessionId is excluded from all clash checks
    // so the session is not flagged as conflicting with its own old slot.
    // Returns "" on success, or an error message on failure.
    std::string validateAndUpdateClassSession(const std::string& editingSessionId,
                                              const ClassSession& updatedSession,
                                              const ConstraintSettings& cs = ConstraintSettings{});

    bool removeClassSession(const std::string& sessionId);

    Instructor*   findInstructorByName(const std::string& name);
    Instructor*   findInstructorById(const std::string& id);
    Course*       findCourseByCode(const std::string& code);
    Room*         findRoomById(const std::string& id);
    StudentBatch* findBatchById(const std::string& id);

    // Deletion helpers — return true if removed
    bool removeInstructor(const std::string& id);
    bool removeCourse(const std::string& code);
    bool removeRoom(const std::string& id);
    bool removeBatch(const std::string& id);

    // Update helpers
    bool updateInstructor(const Instructor& instructor);
    bool updateCourse(const Course& course);
    bool updateRoom(const Room& room);
    bool updateBatch(const StudentBatch& batch);

    // Usage checks — block deletion if referenced
    bool isInstructorUsed(const std::string& id) const;
    bool isCourseUsed(const std::string& code) const;
    bool isRoomUsed(const std::string& id) const;
    bool isBatchUsed(const std::string& id) const;

    // Helper to count how many sessions an instructor is currently assigned to (for manual editing checks)
    // One session equals 1 allocated hour. skipSessionId ignores a specific session (useful when editing a session in-place).
    int countInstructorScheduledHours(const std::string& instructorId, const std::string& skipSessionId = "") const;

    const std::vector<ClassSession>&   getTimetable()    const;
    const std::vector<Instructor>&     getInstructors()  const;
    const std::vector<Course>&         getCourses()      const;
    const std::vector<Room>&           getRooms()        const;
    const std::vector<StudentBatch>&   getBatches()      const;

    // Auto-generation — now driven by ConstraintSettings
    void clearTimetable();
    void clearAllData();

    std::string autoGenerateTimetable(const ConstraintSettings& cs = ConstraintSettings{});

    // Busy-check helpers (used by generator and by the UI session form)
    bool isInstructorBusyAt(const std::string& id, const TimeSlot& slot) const;
    bool isRoomBusyAt(const std::string& id,       const TimeSlot& slot) const;
    bool isBatchBusyAt(const std::string& id,      const TimeSlot& slot) const;
};

#endif // APP_MANAGER_HPP
