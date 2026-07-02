#include "timeslot.hpp"
TimeSlot::TimeSlot(Day day,ClockTime startTime,ClockTime endTime){
    m_day=day;
    m_start_time=startTime;
    m_end_time=endTime;

}
Day TimeSlot::getDay() const{
    return m_day;
}
ClockTime TimeSlot::getStartTime() const{
    return m_start_time;
}
ClockTime TimeSlot::getEndTime() const{
    return m_end_time;
}
int TimeSlot::getDurationmin() const{
    int duration=(m_end_time.hours*60+m_end_time.minutes)-(m_start_time.hours*60+m_start_time.minutes);
    return duration;
}

bool TimeSlot::overlapsWith(const TimeSlot& other) const {
    int thisStart = convertToMinutes(m_start_time);
    int thisEnd = convertToMinutes(m_end_time);
    int otherStart = convertToMinutes(other.getStartTime());
    int otherEnd = convertToMinutes(other.getEndTime());

    // Returns true if the intervals cross over each other
    return (thisStart < otherEnd) && (otherStart < thisEnd);
}