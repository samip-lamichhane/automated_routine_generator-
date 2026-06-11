#include "timeslot.hpp"
#include "classSession.hpp" // Fixed: Standardized filename casing

// Fixed: Reordered initializer list to perfectly match the class declaration order (room before batch)
ClassSession::ClassSession(TimeSlot timeslot, Instructor teacherid, Course subjectid, Room roomid, StudentBatch batchid)
    : m_timeslot(timeslot), m_teacherid(teacherid), m_subjectid(subjectid), m_roomid(roomid), m_batchid(batchid)
{
}

TimeSlot ClassSession::getTimeSlot() const {
    return m_timeslot;
}

Instructor ClassSession::getTeacherId() const {
    return m_teacherid;
}

Course ClassSession::getSubjectId() const {
    return m_subjectid;
}

Room ClassSession::getRoomId() const {
    return m_roomid;
}

StudentBatch ClassSession::getBatchId() const {
    return m_batchid;
}