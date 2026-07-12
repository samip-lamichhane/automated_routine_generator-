#ifndef TIMESLOT_H
#define TIMESLOT_H
#include<string>
enum class Day{
    Monday,Tuesday,Wednesday,Thursday,Friday
    
};
struct ClockTime{
    int hours;
    int minutes;
};
class TimeSlot{
    private:
    Day m_day;
    ClockTime m_start_time;
    ClockTime m_end_time;
public:
TimeSlot(Day day,ClockTime startTime,ClockTime endTime);
Day getDay() const;
ClockTime getStartTime() const;
ClockTime getEndTime() const;
int getDurationmin() const;
};
#endif
