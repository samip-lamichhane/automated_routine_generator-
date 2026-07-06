#include <iostream>
#include <vector>
#include <string>
#include "timeslot.hpp"
#include "classSession.hpp" 
#include "Instructor.hpp"
#include "Room.hpp"
#include "Student_batch.hpp"
#include "Course.hpp"
#include "schedule_validator.hpp" // Added from your friend's code

int main() {
    std::cout << "=== University Routine Generator: Week 1 Integration ===\n" << std::endl;
    
    // --- 1. YOUR ORIGINAL CODE: Verification Test ---
    ClockTime startTime{9, 30};
    ClockTime endTime{11, 30};
    TimeSlot slot1(Day::Monday, startTime, endTime);

    Course subject("COMP-102", 3);
    Instructor teacher("Dr. Niraj Sharma", 12);

    std::cout << "--- Testing Instructor Tracking ---" << std::endl;
    if (teacher.assignNewCourse(subject)) {
        std::cout << "Success: Assigned " << subject.getCourseCode() << " to " << teacher.getName() << "\n" << std::endl;
    }

    Room classroom("Block-C-102", 60, RoomType::Theory);
    StudentBatch batch("BCT-2025-A", 48, ProgramType::BCE);

    ClassSession session1(slot1, &teacher, &subject, &classroom, &batch);

    std::cout << "--- Final Routine Entry Verification ---" << std::endl;
    std::cout << "Course Code: " << session1.getSubjectId()->getCourseCode() << std::endl;
    std::cout << "Instructor:  " << session1.getTeacherId()->getName() << std::endl;
    std::cout << "Room:        " << session1.getRoomId()->getRoomId() << " (Capacity: " << session1.getRoomId()->getCapacity() << ")" << std::endl;
    std::cout << "Batch:       " << session1.getBatchId()->getBatchId() 
              << " (Program: " << session1.getBatchId()->getProgramAsString() 
              << ", Strength: " << session1.getBatchId()->getStrength() << " students)" << std::endl;
    std::cout << "Duration:    " << session1.getTimeSlot().getDurationmin() << " minutes\n" << std::endl;
    
    
    // --- 2. YOUR FRIEND'S CODE: Clash Validation Test ---
    std::cout << "--- Testing Schedule Validator ---" << std::endl;

    // Create Overlapping TimeSlots
    ClockTime start1{9, 0};   // 09:00 AM
    ClockTime end1{10, 30};  // 10:30 AM
    ClockTime start2{10, 0};  // 10:00 AM (Overlaps!)
    ClockTime end2{11, 30};  // 11:30 AM

    TimeSlot slotA(Day::Monday, start1, end1);
    TimeSlot slotB(Day::Monday, start2, end2);

    // Setup two clashing sessions sharing the same Instructor (Dr. Niraj Sharma)
    Course math("COMP101", 3);
    Course physics("PHYS101", 3);

    // Reusing the teacher, room, and batch pointers matching your class layout
    ClassSession clashSession1(slotA, &teacher, &math, &classroom, &batch);
    ClassSession clashSession2(slotB, &teacher, &physics, &classroom, &batch); 

    std::vector<ClassSession> testTimetable = { clashSession1, clashSession2 };

    // Run Core Validation Checks
    std::string errorMsg;
    if (ScheduleValidator::hasInstructorClash(testTimetable, errorMsg)) {
        std::cout << "SUCCESS: Clash caught cleanly!" << std::endl;
        std::cout << "Error message: " << errorMsg << std::endl;
    } else {
        std::cout << "FAILURE: Validator missed the instructor clash." << std::endl;
    }

    return 0;
}