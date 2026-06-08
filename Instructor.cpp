#include "Instructor.hpp"
#include <iostream>

Instructor::Instructor(std::string instructorName, int maxHours) {
    name = instructorName;
    maxLimitHours = maxHours;
}

std::string Instructor::getName() const {
    return name;
}

// Logic to count total hours assigned across the whole schedule so far
int Instructor::calculateTotalAssignedHours() const {
    int total = 0;
    // Loops through every course currently in the vector and adds up the hours
    for (const Course& c : assignedCourses) {
        total += c.getAllocatedHours();
    }
    return total;
}

// Logic to make sure a subject/workload doesn't exceed boundaries
bool Instructor::assignNewCourse(const Course& newCourse) {
    int currentHours = calculateTotalAssignedHours();
    
    // If adding this new course goes over the limit, reject it!
    if (currentHours + newCourse.getAllocatedHours() > maxLimitHours) {
        std::cout << "Error: Cannot assign " << newCourse.getCourseCode() 
                  << ". It exceeds " << name << "'s weekly hour limit!\n";
        return false; // Assignment failed
    }

    // Otherwise, successfully add it to the schedule tracking list
    assignedCourses.push_back(newCourse);
    return true; // Assignment succeeded
}
