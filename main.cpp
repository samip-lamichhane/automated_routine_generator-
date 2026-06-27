#include <iostream>
#include <string>
#include "timeslot.hpp"
#include "classSession.hpp" 
#include "Instructor.hpp"
#include "Room.hpp"
#include "Student_batch.hpp"
#include "Course.hpp"

int main() {
    std::cout << "=== University Routine Generator: Week 1 Integration ===\n" << std::endl;

    
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
    std::cout << "Duration:    " << session1.getTimeSlot().getDurationmin() << " minutes" << std::endl;
    
    
    return 0;
}