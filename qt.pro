QT += widgets

CONFIG += c++17

INCLUDEPATH += ../..

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    CRUD.cpp \
    AppManager.cpp \
    Course.cpp \
    Instructor.cpp \
    room.cpp \
    Student_batch.cpp \
    classSession.cpp \
    timeslot.cpp \
    schedule_validator.cpp \
    mainwindow_helper.cpp

HEADERS += \
    mainwindow.h \
    CRUD.h \
    AppManager.hpp \
    ConstraintSettings.hpp \
    Course.hpp \
    Instructor.hpp \
    room.hpp \
    Student_batch.hpp \
    classSession.hpp \
    timeslot.hpp \
    schedule_validator.hpp \
    mainwindow_helper.hpp

FORMS += \
    mainwindow.ui