#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QTimeEdit>
#include <QCheckBox>
#include <QVector>

#include "AppManager.hpp"
#include "ConstraintSettings.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class CRUDManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class CRUDManager;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onSubjectCountChanged(int count);  // rebuilds the dynamic subject dropdowns
    void onAutoGenerate();
    void onViewBatchChanged(); // Triggers a redraw of the timetable grid
    void onRefreshGridClicked(); // Manually refreshes the grid
    void onGridCellClicked(int row, int col); // Edit-in-place: handles occupied/empty cell clicks

    // ── Constraints tab slots ────────────────────────────────────────────────
    void onValidateConstraints();
    void onConstraintsChanged();   // resets validated flag, disables generate button

private:
    Ui::MainWindow *ui;
    AppManager m_appManager;
    CRUDManager* m_crudManager;

    // ── Constraint state ─────────────────────────────────────────────────────
    bool               m_constraintsValidated = false;
    ConstraintSettings m_constraints;

    // ── Helper methods ────────────────────────────────────────────────────────
    void setupSessionDialog();     // builds the Add/Edit session QDialog
    void connectSignals();         // wires all signal/slot connections
    void applyDynamicStyles();     // applies per-widget stylesheets after setupUi
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
    void openSessionDialogForEdit(const std::string& sessionId);
    void openSessionDialogForAdd(int prefillDayRow = -1, int prefillColSlot = -1);
    void resetSessionDialogToAddMode();

    // ── Dynamic subject combos (created at runtime inside instSubjectContainer) ──
    QVBoxLayout          *m_instSubjectLayout; // obtained from ui->instSubjectContainer->layout()
    QVector<QComboBox*>   m_instSubjectCombos;

    // ── Day checkbox array (mirrors ui->dayCheck0…dayCheck6 for loop access) ──
    QCheckBox *m_dayChecks[7]; // [0]=Sun,[1]=Mon,...,[6]=Sat

    // ── Session dialog (kept fully programmatic — complex edit-mode toggling) ─
    QDialog      *m_addSessionDialog;
    QComboBox    *m_sessInstCombo;
    QComboBox    *m_sessCourseCombo;
    QComboBox    *m_sessRoomCombo;
    QComboBox    *m_sessBatchCombo;
    QComboBox    *m_sessDayCombo;
    QTimeEdit    *m_sessStartEdit;
    QTimeEdit    *m_sessEndEdit;

    // ── Session dialog buttons (promoted to members for Edit-mode toggling) ───
    QPushButton  *m_btnDialogSchedule; // "Schedule Class Session" / "Save Changes"
    QPushButton  *m_btnDialogDelete;   // "Delete This Session" — only visible in Edit mode

    // ── Edit-in-place state ───────────────────────────────────────────────────
    QString       m_editingSessionId;  // empty = Add mode, non-empty = Edit mode

    // ── Editing states ────────────────────────────────────────────────────────
    std::string m_editingInstId;
    std::string m_editingCourseCode;
    std::string m_editingRoomId;
    std::string m_editingBatchId;
};

#endif // MAINWINDOW_H
