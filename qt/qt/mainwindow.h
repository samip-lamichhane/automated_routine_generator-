#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QTableWidget>
#include <QLabel>
#include <QTimeEdit>
#include <QMessageBox>

#include "AppManager.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAddInstructor();
    void onAddCourse();
    void onAddRoom();
    void onAddBatch();
    void onAddClassSession();

private:
    Ui::MainWindow *ui;
    AppManager m_appManager;

    // Helper UI setup
    void setupUI();
    void populateCombos();
    void refreshListsAndTables();
    void populateInitialData();
    void saveToFile();
    void loadFromFile();

    // Tab widgets
    QTabWidget *m_tabWidget;

    // Instructor UI
    QLineEdit *m_instNameEdit;
    QSpinBox *m_instHoursSpin;
    QListWidget *m_instList;

    // Course UI
    QLineEdit *m_courseCodeEdit;
    QSpinBox *m_courseHoursSpin;
    QListWidget *m_courseList;

    // Room UI
    QLineEdit *m_roomIdEdit;
    QSpinBox *m_roomCapSpin;
    QComboBox *m_roomTypeCombo;
    QListWidget *m_roomList;

    // Batch UI
    QLineEdit *m_batchIdEdit;
    QSpinBox *m_batchStrengthSpin;
    QComboBox *m_batchProgCombo;
    QListWidget *m_batchList;

    // Session UI
    QComboBox *m_sessInstCombo;
    QComboBox *m_sessCourseCombo;
    QComboBox *m_sessRoomCombo;
    QComboBox *m_sessBatchCombo;
    QComboBox *m_sessDayCombo;
    QTimeEdit *m_sessStartEdit;
    QTimeEdit *m_sessEndEdit;
    QTableWidget *m_timetableTable;
};

#endif // MAINWINDOW_H
