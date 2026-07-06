#ifndef APP_MANAGER_HPP
#define APP_MANAGER_HPP
#include <vector>
#include <string>
#include "Instructor.hpp"
#include "Course.hpp"
#include "Room.hpp"
#include "Student_batch.hpp"
#include "classSession.hpp"
#include <fstream>
class AppManager{
    private:
    std::vector<Instructor> m_masterInstructors;
    std::vector<Course> m_masterCourses;
    std::vector<Room> m_masterRooms;
    std::vector<StudentBatch> m_masterBatches;
    
    std::vector<ClassSession> m_timetable; //Final generated Container
    public:
    AppManager()= default;
    //samip ko qt le call garna parxa
    void addInstructor(const Instructor& instructor);
    void addRoom(const Room& room);
    void addCourse(const Course& course);
    void addBatch(const StudentBatch& batch);
    void addClassSession(const ClassSession& session);

    Instructor* findInstructorByName(const std::string& name);
    Course* findCourseByCode(const std::string& code);
    Room* findRoomById(const std::string& id);
    StudentBatch* findBatchById(const std::string& id);

    
    const std::vector<ClassSession>& getTimetable() const;
    bool saveSystemData(const std::string& filename);
    bool loadSystemData(const std::string& filename);

} ;
#endif
