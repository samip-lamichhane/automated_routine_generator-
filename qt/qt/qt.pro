QT += widgets

CONFIG += c++17

INCLUDEPATH += ../..

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    ../../AppManager.cpp \
    ../../Course.cpp \
    ../../Instructor.cpp \
    ../../room.cpp \
    ../../Student_batch.cpp \
    ../../classSession.cpp \
    ../../timeslot.cpp

HEADERS += \
    mainwindow.h \
    ../../AppManager.hpp \
    ../../Course.hpp \
    ../../Instructor.hpp \
    ../../room.hpp \
    ../../Student_batch.hpp \
    ../../classSession.hpp \
    ../../timeslot.hpp

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
