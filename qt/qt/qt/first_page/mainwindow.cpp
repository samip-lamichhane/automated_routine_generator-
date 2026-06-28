#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Make table columns distribute evenly across screen space
    ui->teacherTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->classroomTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Connect UI layout button to our operational function
    connect(ui->setupButton, &QPushButton::clicked, this, &MainWindow::setupProject);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupProject()
{
    bool ok;

    // ==========================================
    // PHASE 1: CREATE & NAME NEW PROJECT
    // ==========================================
    QString projectName = QInputDialog::getText(this, "Project Initialize",
                                                "Enter the Name of the Project:",
                                                QLineEdit::Normal, "", &ok);
    if (!ok || projectName.isEmpty()) return;

    // Set Window title to your customized Project name
    setWindowTitle(projectName + " - Workspace Environment");

    // ==========================================
    // PHASE 2: DYNAMIC TEACHER ADMISSIONS (WITH SCHEDULE)
    // ==========================================
    int numTeachers = QInputDialog::getInt(this, "Configuration Step 1/2",
                                           "Enter total number of teachers:",
                                           1, 1, 200, 1, &ok);
    if (!ok) return;

    ui->teacherTable->setRowCount(numTeachers);
    for (int i = 0; i < numTeachers; ++i) {
        QString name = QInputDialog::getText(this, "Teacher Details",
                                             QString("Enter Name for Teacher #%1:").arg(i + 1),
                                             QLineEdit::Normal, "", &ok);
        if (!ok) return;

        QString subject = QInputDialog::getText(this, "Teacher Details",
                                                QString("Enter Teaching Subject for %1:").arg(name),
                                                QLineEdit::Normal, "", &ok);
        if (!ok) return;

        QString faculty = QInputDialog::getText(this, "Teacher Details",
                                                QString("Enter Faculty for %1:").arg(name),
                                                QLineEdit::Normal, "", &ok);
        if (!ok) return;

        // NEW: Input for teaching days
        QString days = QInputDialog::getText(this, "Schedule Settings",
                                             QString("Enter Days for %1 (e.g., Monday-Friday):").arg(name),
                                             QLineEdit::Normal, "", &ok);
        if (!ok) return;

        // Populate table row across 5 columns
        ui->teacherTable->setItem(i, 0, new QTableWidgetItem(name));
        ui->teacherTable->setItem(i, 1, new QTableWidgetItem(subject));
        ui->teacherTable->setItem(i, 2, new QTableWidgetItem(faculty));
        ui->teacherTable->setItem(i, 3, new QTableWidgetItem(days));
    }

    // ==========================================
    // PHASE 3: DYNAMIC CLASSROOM ADMISSIONS
    // ==========================================
    int numClassrooms = QInputDialog::getInt(this, "Configuration Step 2/2",
                                             "Enter total number of classrooms:",
                                             1, 1, 200, 1, &ok);
    if (!ok) return;

    ui->classroomTable->setRowCount(numClassrooms);
    for (int i = 0; i < numClassrooms; ++i) {
        QString roomName = QInputDialog::getText(this, "Classroom Space Metrics",
                                                 QString("Enter Name/Number for Classroom #%1:").arg(i + 1),
                                                 QLineEdit::Normal, "", &ok);
        if (!ok) return;

        QString block = QInputDialog::getText(this, "Classroom Space Metrics",
                                              QString("Enter Location/Block for Room %1:").arg(roomName),
                                              QLineEdit::Normal, "", &ok);
        if (!ok) return;

        ui->classroomTable->setItem(i, 0, new QTableWidgetItem(roomName));
        ui->classroomTable->setItem(i, 1, new QTableWidgetItem(block));
    }

    QMessageBox::information(this, "Status Dashboard", "Project initialized and schedule metrics populated successfully!");
}