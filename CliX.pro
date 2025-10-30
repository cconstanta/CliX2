QT += widgets sql

TARGET = CliX
TEMPLATE = app

CONFIG += c++17
CONFIG += console

SOURCES += \
    adminwindow.cpp \
    login.cpp \
    main.cpp \
    masterwindow.cpp \
    ownerwindow.cpp \
    registration.cpp \
    database.cpp \
    employeeswindow.cpp \
    financeswindow.cpp \
    salarywindow.cpp \
    schedulewindow.cpp \
    payrollwindow.cpp \
    workwindow.cpp

HEADERS += \
    adminwindow.h \
    login.h \
    masterwindow.h \
    ownerwindow.h \
    registration.h \
    database.h \
    employeeswindow.h \
    financeswindow.h \
    salarywindow.h \
    schedulewindow.h \
    payrollwindow.h \
    workwindow.h
FORMS +=

RESOURCES += \
    icons/icons.qrc
    win32: {
        CONFIG += windeployqt
        QMAKE_POST_LINK = windeployqt $$OUT_PWD/release/$$TARGET.exe
    }
