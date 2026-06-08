#ifndef INSTRUCTOR_HPP
#define INSTRUCTOR_HPP

#include <string>
#include <vector>
#include "Course.hpp" // Links Course to Instructor

class Instructor {
private:
    std::string name;
    int maxLimitHours;
    std::vector<Course> assignedCourses; // Tracking assigned courses

public:
    Instructor(std::string instructorName, int maxHours);

    std::string getName() const;
    
    // The logic functions you need to adapt for your tracking challenge:
    int calculateTotalAssignedHours() const;
    bool assignNewCourse(const Course& newCourse);
};

#endif
