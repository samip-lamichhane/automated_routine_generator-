#ifndef CLASSSSESSION_HPP
#define CLASSSSESSION_HPP
#include "timeslot.hpp"
#include <string>

class ClassSession{
private:
TimeSlot m_timeslot;
std::string m_teacherid;
std::string m_subjectid;
std::string m_roomid;
std::string m_batchid;
public:
ClassSession(TimeSlot timeslot,std::string teacherid,std::string subjectid,std::string roomid,std::string batchid );
TimeSlot getTimeSlot() const;
std::string getTeacherId() const;
std::string getSubjectId() const;
std::string getRoomId() const;
std::string getBatchId() const;



};
#endif
