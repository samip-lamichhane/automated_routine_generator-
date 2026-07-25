#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
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
#include <QVector>
#include <QScrollArea>
#include <QCheckBox>
#include <QGroupBox>
#include <QTextEdit>

#include "AppManager.hpp"
#include "ConstraintSettings.hpp"

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
    void onEditInstructor();
    void onDeleteInstructor();
    void onSubjectCountChanged(int count);  // rebuilds the dynamic subject dropdowns
    void onAddCourse();
    void onEditCourse();
    void onDeleteCourse();
    void onAddRoom();
    void onEditRoom();
    void onDeleteRoom();
    void onAddBatch();
    void onEditBatch();
    void onDeleteBatch();
    void onAddClassSession();
    void onDeleteClassSession();
    void onAutoGenerate();
    void onViewBatchChanged(); // Triggers a redraw of the timetable grid
    void onRefreshGridClicked(); // Manually refreshes the grid
    void onGridCellClicked(int row, int col); // Edit-in-place: handles occupied/empty cell clicks
    void onResetAllData(); // Global reset

    // ── Constraints tab slots ────────────────────────────────────────────────
    void onValidateConstraints();
    void onConstraintsChanged();   // resets validated flag, disables generate button

private:
    Ui::MainWindow *ui;
    AppManager m_appManager;

    // ── Constraint state ─────────────────────────────────────────────────────
    bool              m_constraintsValidated = false;
    ConstraintSettings m_constraints;

    // ── Helper methods ────────────────────────────────────────────────────────
    void setupUI();
    void setupConstraintsTab();
    void populateCombos();
    void refreshListsAndTables();
    void refreshTimetableGrid();
    void refreshInstList();
    void populateInitialData();
    void saveToFile();
    void loadFromFile();
    void rebuildSubjectCombos(int count);
    void markConstraintsDirty();       // sets m_constraintsValidated=false, disables generate btn
    void updateCapacityLabel();        // recomputes and updates the live capacity label
    ConstraintSettings readConstraintsFromUI() const;

    // ── Edit-in-place helpers ─────────────────────────────────────────────────
    // Opens the session dialog pre-filled with the session identified by sessionId (Edit mode).
    void openSessionDialogForEdit(const std::string& sessionId);
    // Opens the session dialog for a new session, optionally pre-filling day/time
    // from the clicked empty grid cell (row = day index, col = time-slot index).
    void openSessionDialogForAdd(int prefillDayRow = -1, int prefillColSlot = -1);
    // Resets the session dialog to a clean Add-mode state (clears m_editingSessionId,
    // restores button labels, hides Delete button).
    void resetSessionDialogToAddMode();

    // ── Tab widget ────────────────────────────────────────────────────────────
    QTabWidget *m_tabWidget;

    // ── Instructor UI ─────────────────────────────────────────────────────────
    QLineEdit   *m_instIdEdit;
    QLineEdit   *m_instNameEdit;
    QSpinBox    *m_instHoursSpin;
    QSpinBox    *m_instSubjectCountSpin;
    QWidget     *m_instSubjectContainer;
    QVBoxLayout *m_instSubjectLayout;
    QVector<QComboBox*> m_instSubjectCombos;
    QListWidget *m_instList;

    // ── Course UI ─────────────────────────────────────────────────────────────
    QLineEdit   *m_courseCodeEdit;
    QLineEdit   *m_courseNameEdit;
    QSpinBox    *m_courseHoursSpin;
    QListWidget *m_courseList;

    // ── Room UI ───────────────────────────────────────────────────────────────
    QLineEdit *m_roomIdEdit;
    QLineEdit *m_roomBuildingEdit;
    QSpinBox  *m_roomCapSpin;
    QComboBox *m_roomTypeCombo;
    QListWidget *m_roomList;

    // ── Batch UI ──────────────────────────────────────────────────────────────
    QLineEdit *m_batchIdEdit;
    QSpinBox  *m_batchStrengthSpin;
    QComboBox *m_batchProgCombo;
    QLineEdit *m_batchDeptEdit;
    QListWidget *m_batchList;

    // ── Session / Generate UI ─────────────────────────────────────────────────
    QComboBox    *m_sessInstCombo;
    QComboBox    *m_sessCourseCombo;
    QComboBox    *m_sessRoomCombo;
    QComboBox    *m_sessBatchCombo;
    QComboBox    *m_sessDayCombo;
    QTimeEdit    *m_sessStartEdit;
    QTimeEdit    *m_sessEndEdit;
    QComboBox    *m_viewBatchCombo;    // Dropdown to select batch for visual grid
    QTabWidget   *m_timetableSubTabs;  // Holds Schedule and Grid views
    QTableWidget *m_timetableTable;    // Flat row-by-row table (Schedule)
    QTableWidget *m_timetableGrid;     // Visual grid
    QPushButton  *m_btnRefreshGrid;    // Refreshes visual grid
    QPushButton  *m_btnAutoGenerate;   // kept as member so we can enable/disable it
    QPushButton  *m_btnResetAllData;   // Global clear button
    QDialog      *m_addSessionDialog;  // Dialog for adding/editing class sessions

    // ── Session dialog buttons (promoted to members for Edit-mode toggling) ───
    QPushButton  *m_btnDialogSchedule; // "Schedule Class Session" / "Save Changes"
    QPushButton  *m_btnDialogDelete;   // "Delete This Session" — only visible in Edit mode

    // ── Edit-in-place state ───────────────────────────────────────────────────
    QString       m_editingSessionId;  // empty = Add mode, non-empty = Edit mode

    // ── Constraints tab UI ────────────────────────────────────────────────────
    // Working days
    QCheckBox *m_dayChecks[7];         // [0]=Sun,[1]=Mon,...,[6]=Sat
    // Time window
    QTimeEdit *m_dayStartEdit;
    QTimeEdit *m_dayEndEdit;
    // Lunch break
    QCheckBox *m_lunchEnabledCheck;
    QTimeEdit *m_lunchStartEdit;
    QTimeEdit *m_lunchEndEdit;
    // Live capacity display
    QLabel    *m_capacityLabel;
    // Rule checkboxes
    QCheckBox *m_ruleNoInstDoubleBook;
    QCheckBox *m_ruleNoRoomDoubleBook;
    QCheckBox *m_ruleNoBatchClash;
    QCheckBox *m_ruleInstDayGap;
    QCheckBox *m_ruleNoSameSubjectConsec;
    QCheckBox *m_ruleMaxWeeklyHours;
    QCheckBox *m_ruleMaxConsecHoursEnabled;
    QSpinBox  *m_maxConsecHoursSpin;
    QCheckBox *m_ruleSubjectLock;      // always checked, disabled (read-only)
    // Validation output
    QTextEdit   *m_validationOutput;

    // ── Editing states ────────────────────────────────────────────────────────
    std::string m_editingInstId;
    std::string m_editingCourseCode;
    std::string m_editingRoomId;
    std::string m_editingBatchId;
};

#endif // MAINWINDOW_H
