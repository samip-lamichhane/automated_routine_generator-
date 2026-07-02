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
bool AppManager::saveSystemData(const std::string& filename){
std::ofstream outFile(filename);
if(!outFile.is_open()) return false;//failed to create or open file 
for(size_t i=0;i<m_masterInstructors.size();++i){
    outFile<<"INSTRUCTOR|"<<m_masterInstructors[i].getName()<<"|"<<m_masterInstructors[i].calculateTotalAssignedHours()<<"\n";

}
for(size_t i=0;i<m_masterRooms.size();++i){
    outFile<<"ROOM|"<<m_masterRooms[i].getRoomId()<<"|"<<m_masterRooms[i].getCapacity()<<"\n";

}
for(size_t i=0;i<m_masterCourses.size();++i){
    outFile<<"COURSES|"<<m_masterCourses[i].getCourseCode()<<"|"<<m_masterCourses[i].getAllocatedHours()<<"\n";

}
for(size_t i=0;i<m_masterBatches.size();++i){
    outFile<<"ROOM|"<<m_masterBatches[i].getBatchId()<<"|"<<m_masterBatches[i].getProgramAsString()<<"\n";
}

}