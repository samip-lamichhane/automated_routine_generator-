#include "AppManager.hpp"
void AppManager::addInstructor(const Instructor& instructor){
    m_masterInstructors.push_back(instructor);
}
void AppManager::addRoom(const Room& room){
    m_masterRooms.push_back(room);
}
void AppManager::addCourse(const Course& course){
    m_masterCourses.push_back(course);
}
void AppManager::addBatch(const StudentBatch& batch ){
    m_masterBatches.push_back(batch);

}
void AppManager::addClassSession(const ClassSession& session){
    m_timetable.push_back(session);
}
Instructor* AppManager::findInstructorByName(const std::string& name){
 for(size_t i=0;i<m_masterInstructors.size(); ++i){
    if(m_masterInstructors[i].getName()==name){
        return &m_masterInstructors[i];

    }
 }
 return nullptr;
}
Course* AppManager::findCourseByCode(const std::string& code){
    for(size_t i=0;i<m_masterCourses.size();++i){
       if(m_masterCourses[i].getCourseCode()==code){
        return &m_masterCourses[i];
       }
    }
    return nullptr;
}
StudentBatch* AppManager::findBatchById(const std::string& id){
    for(size_t i=0;i<m_masterBatches.size();i++){
        if(m_masterBatches[i].getBatchId()==id ){
            return &m_masterBatches[i];
        }
    }
    return nullptr;
}
Room* AppManager::findRoomById(const std::string& id){
    for(size_t i=0;i<m_masterRooms.size();i++){
        if(m_masterRooms[i].getRoomId()==id){
            return &m_masterRooms[i];

        }
    }
    return nullptr;

}
const std::vector<ClassSession>& AppManager::getTimetable() const{
    return m_timetable;
}
const std::vector<Instructor>& AppManager::getInstructors() const {
    return m_masterInstructors;
}
const std::vector<Course>& AppManager::getCourses() const {
    return m_masterCourses;
}
const std::vector<Room>& AppManager::getRooms() const {
    return m_masterRooms;
}
const std::vector<StudentBatch>& AppManager::getBatches() const {
    return m_masterBatches;
}