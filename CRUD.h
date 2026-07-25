#ifndef CRUD_H
#define CRUD_H

#include <QObject>

class MainWindow;

class CRUDManager : public QObject
{
    Q_OBJECT
public:
    explicit CRUDManager(MainWindow *mainWindow, QObject *parent = nullptr);

public slots:
    void onAddInstructor();
    void onEditInstructor();
    void onDeleteInstructor();

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

    void onResetAllData();

private:
    MainWindow *m_mainWindow;
};

#endif // CRUD_H
