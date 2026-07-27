#ifndef TIMESLOT_H
#define TIMESLOT_H
#include <string>

// Sunday=5 and Saturday=6 are the new values for the two extra days.
enum class Day {
    Monday=0, Tuesday, Wednesday, Thursday, Friday,
    Sunday=5, Saturday=6
};

struct ClockTime {
    int hours;
    int minutes;
};

class TimeSlot {
private:
    Day m_day;
    ClockTime m_start_time;
    ClockTime m_end_time;

public:
    TimeSlot(Day day, ClockTime startTime, ClockTime endTime);

    Day       getDay()       const;
    ClockTime getStartTime() const;
    ClockTime getEndTime()   const;
    int       getDurationmin() const;

    bool overlapsWith(const TimeSlot& other) const;

    // Returns the overlap (in minutes) between this slot and a configurable
    // Defaults to the legacy hard-coded 11:00–12:00 window when called with
    // no arguments so that existing call sites remain valid.
    int lunchBreakOverlapMinutes(int lunchStartMin = 11 * 60,
                                  int lunchEndMin   = 12 * 60) const;
};

#endif
