#include "timeslot.hpp"

TimeSlot::TimeSlot(Day day, ClockTime startTime, ClockTime endTime)
    : m_day(day), m_start_time(startTime), m_end_time(endTime)
{}

Day TimeSlot::getDay() const {
    return m_day;
}

ClockTime TimeSlot::getStartTime() const {
    return m_start_time;
}

ClockTime TimeSlot::getEndTime() const {
    return m_end_time;
}

int TimeSlot::getDurationmin() const {
    return (m_end_time.hours * 60 + m_end_time.minutes)
         - (m_start_time.hours * 60 + m_start_time.minutes);
}

bool TimeSlot::overlapsWith(const TimeSlot& other) const {
    if (m_day != other.getDay()) return false;

    int thisStart  = m_start_time.hours * 60 + m_start_time.minutes;
    int thisEnd    = m_end_time.hours   * 60 + m_end_time.minutes;
    int otherStart = other.getStartTime().hours * 60 + other.getStartTime().minutes;
    int otherEnd   = other.getEndTime().hours   * 60 + other.getEndTime().minutes;

    return (thisStart < otherEnd) && (otherStart < thisEnd);
}

// Configurable lunch-break overlap check.
// lunchStartMin and lunchEndMin are minutes from midnight (e.g. 13*60 = 780).
int TimeSlot::lunchBreakOverlapMinutes(int lunchStartMin, int lunchEndMin) const {
    int thisStart = m_start_time.hours * 60 + m_start_time.minutes;
    int thisEnd   = m_end_time.hours   * 60 + m_end_time.minutes;

    int overlapStart = (thisStart > lunchStartMin) ? thisStart : lunchStartMin;
    int overlapEnd   = (thisEnd   < lunchEndMin)   ? thisEnd   : lunchEndMin;

    return (overlapStart < overlapEnd) ? (overlapEnd - overlapStart) : 0;
}
