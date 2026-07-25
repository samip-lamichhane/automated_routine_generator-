#ifndef MAINWINDOW_HELPER_HPP
#define MAINWINDOW_HELPER_HPP

#include <QString>
#include <QMap>
#include <QColor>
#include <QJsonObject>
#include <vector>

#include "timeslot.hpp"
#include "Instructor.hpp"
#include "Course.hpp"
#include "ConstraintSettings.hpp"

class AppManager;
class QMainWindow;
class QCheckBox;
namespace Ui { class MainWindow; }

namespace MainWindowHelper {
    // String and conversion helpers
    QString dayToString(Day d);
    QString formatClockTime(ClockTime t);
    QString instDisplayString(const Instructor& inst);
    QMap<QString, QColor> buildCourseColorMap(const std::vector<Course>& courses);

    // Initial data setup
    void populateInitialData(AppManager& appManager);

    // UI Style and Setup
    void applyGlobalStyle(QMainWindow* window);
    void applyDynamicStyles(QMainWindow* window, Ui::MainWindow* ui, QCheckBox** dayChecks);

    // Timetable grid rendering
    void refreshTimetableGrid(Ui::MainWindow* ui, const AppManager& appManager, const ConstraintSettings& cs, const QString& batchId);

    // File I/O
    void saveTimetableData(const QString& filePath, const AppManager& appManager, const ConstraintSettings& cs);
    QJsonObject loadTimetableData(const QString& filePath, AppManager& appManager, bool& outLoaded);
}

#endif // MAINWINDOW_HELPER_HPP
