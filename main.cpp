#include <iostream>
#include "Course.hpp"
#include "Instructor.hpp"

int main() {
    // Create an Instructor with a strict limit of 6 teaching hours per week
    Instructor profSmith("Dr. Smith", 6);

    // Create a few courses with different hourly workloads
    Course c++Basics("CS101", 3);
    Course dataStructures("CS202", 3);
    Course advancedAlgorithms("CS303", 2);

    std::cout << "--- Attempting Schedule Allocations ---\n";

    // 1. Try assigning the first 3-hour course (Total becomes 3/6)
    profSmith.assignNewCourse(c++Basics);

    // 2. Try assigning the second 3-hour course (Total becomes 6/6)
    profSmith.assignNewCourse(dataStructures);

    std::cout << "Current Assigned Hours: " << profSmith.calculateTotalAssignedHours() << " hours.\n";

    // 3. Try assigning another 2-hour course (Total would be 8/6 -> This should fail!)
    profSmith.assignNewCourse(advancedAlgorithms);

    std::cout << "Final Total Hours: " << profSmith.calculateTotalAssignedHours() << " hours.\n";

    return 0;
}
