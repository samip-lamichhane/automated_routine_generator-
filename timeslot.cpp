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
