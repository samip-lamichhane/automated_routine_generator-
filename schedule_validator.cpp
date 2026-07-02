#include "schedule_validator.hpp"

bool ScheduleValidator::hasInstructorClash(const std::vector<ClassSession>& allSessions, std::string& outErrorMessage) {
    for (size_t i = 0; i < allSessions.size(); ++i) {
        for (size_t j = i + 1; j < allSessions.size(); ++j) {
            
            // Extract the instructor pointers from the ClassSession objects
            const Instructor* teacher1 = allSessions[i].getTeacherId();
            const Instructor* teacher2 = allSessions[j].getTeacherId();
            
            // Safety guard: skip if either session doesn't have an instructor assigned yet
            if (teacher1 == nullptr || teacher2 == nullptr) {
                continue;
            }
            
            // Compare the memory addresses (pointers) directly
            if (teacher1 == teacher2) {
                // Check if they occur on the same day
                if (allSessions[i].getTimeSlot().getDay() == allSessions[j].getTimeSlot().getDay()) {
                    // Check if their execution intervals overlap
                    if (allSessions[i].getTimeSlot().overlapsWith(allSessions[j].getTimeSlot())) {
                        
                        const Course* subject1 = allSessions[i].getSubjectId();
                        const Course* subject2 = allSessions[j].getSubjectId();
                        
                        std::string course1 = (subject1 != nullptr) ? subject1->getCourseCode() : "Unknown Course";
                        std::string course2 = (subject2 != nullptr) ? subject2->getCourseCode() : "Unknown Course";

                        // Formulate the error message for the Qt UI pop-up window
                        outErrorMessage = "Instructor Clash! " + teacher1->getName() +
                                          " is double-booked for " + course1 + " and " + course2 + ".";
                        return true; 
                    }
                }
            }
        }
    }
    return false; 
}

bool ScheduleValidator::hasRoomClash(const std::vector<ClassSession>& allSessions, std::string& outErrorMessage) {
    for (size_t i = 0; i < allSessions.size(); ++i) {
        for (size_t j = i + 1; j < allSessions.size(); ++j) {
            
            // Extract the Room pointers
            const Room* room1 = allSessions[i].getRoomId();
            const Room* room2 = allSessions[j].getRoomId();
            
            // Safety guard: skip if either session doesn't have a room assigned yet
            if (room1 == nullptr || room2 == nullptr) {
                continue;
            }
            
            // Compare the Room pointers directly
            if (room1 == room2) {
                if (allSessions[i].getTimeSlot().getDay() == allSessions[j].getTimeSlot().getDay()) {
                    if (allSessions[i].getTimeSlot().overlapsWith(allSessions[j].getTimeSlot())) {
                        
                        const StudentBatch* batch1 = allSessions[i].getBatchId();
                        const StudentBatch* batch2 = allSessions[j].getBatchId();
                        
                        std::string b1 = (batch1 != nullptr) ? batch1->getBatchId() : "Unknown Batch";
                        std::string b2 = (batch2 != nullptr) ? batch2->getBatchId() : "Unknown Batch";

                        outErrorMessage = "Room Clash! Room " + room1->getRoomId() +
                                          " is occupied by both " + b1 + " and " + b2 + ".";
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool ScheduleValidator::hasBatchClash(const std::vector<ClassSession>& allSessions, std::string& outErrorMessage) {
    for (size_t i = 0; i < allSessions.size(); ++i) {
        for (size_t j = i + 1; j < allSessions.size(); ++j) {
            
            // Extract StudentBatch pointers
            const StudentBatch* batch1 = allSessions[i].getBatchId();
            const StudentBatch* batch2 = allSessions[j].getBatchId();
            
            // Safety guard: skip if either session doesn't have a batch assigned yet
            if (batch1 == nullptr || batch2 == nullptr) {
                continue;
            }
            
            // Compare the StudentBatch pointers directly
            if (batch1 == batch2) {
                if (allSessions[i].getTimeSlot().getDay() == allSessions[j].getTimeSlot().getDay()) {
                    if (allSessions[i].getTimeSlot().overlapsWith(allSessions[j].getTimeSlot())) {
                        
                        const Course* subject1 = allSessions[i].getSubjectId();
                        const Course* subject2 = allSessions[j].getSubjectId();
                        
                        std::string course1 = (subject1 != nullptr) ? subject1->getCourseCode() : "Unknown Course";
                        std::string course2 = (subject2 != nullptr) ? subject2->getCourseCode() : "Unknown Course";

                        outErrorMessage = "Batch Clash! Student Batch " + batch1->getBatchId() +
                                          " has overlapping sessions for " + course1 + " and " + course2 + ".";
                        return true;
                    }
                }
            }
        }
    }
    return false; 
}