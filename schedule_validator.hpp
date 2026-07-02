#ifndef SCHEDULE_VALIDATOR_HPP
#define SCHEDULE_VALIDATOR_HPP

#include "classSession.hpp"
#include <vector>
#include <string>

class ScheduleValidator {
public:
    // Rule 1: Checks if the same instructor is booked for two different courses at overlapping times
    static bool hasInstructorClash(const std::vector<ClassSession>& allSessions, std::string& outErrorMessage);

    // Rule 2: Checks if a student section is scheduled for two classes at once
    static bool hasBatchClash(const std::vector<ClassSession>& allSessions, std::string& outErrorMessage);

    // Rule 3: Checks if two classes are competing for the exact same room
    static bool hasRoomClash(const std::vector<ClassSession>& allSessions, std::string& outErrorMessage);
};

#endif // SCHEDULE_VALIDATOR_HPP