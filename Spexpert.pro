QT       += core gui
QT       += printsupport axcontainer serialport

# INCLUDEPATH   += c:\Programy\mingw\mingw32\i686-w64-mingw32\include\ddk

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = spexpert
TEMPLATE = app

HEADERS += \
    winspecvb.h \
    mainwindow.h \
    centralwidget.h \
    exptaskscheduler.h \
    appcore.h \
    exptask.h \
    winspec.h \
    exptasklist.h \
    appstate.h \
    lockableqvector.h \
    qcustomplot/qcustomplot.h \
    timespan.h \
    waittasklist.h \
    waittask.h \
    exptasks.h \
    waittasks.h \
    usmcvb_com.h \
    stagecontrol.h \
    plotproxy.h \
    neslabusmainwidget.h \
    neslab.h \
    experimentsetup.h \
    stagesetup.h

SOURCES += \
    winspecvb.cpp \
    main.cpp \
    mainwindow.cpp \
    centralwidget.cpp \
    appcore.cpp \
    exptask.cpp \
    winspec.cpp \
    exptasklist.cpp \
    appstate.cpp \
    qcustomplot/qcustomplot.cpp \
    lockableqvector.cpp \
    timespan.cpp \
    waittasklist.cpp \
    waittask.cpp \
    exptasks.cpp \
    waittasks.cpp \
    usmcvb_com.cpp \
    stagecontrol.cpp \
    plotproxy.cpp \
    neslab.cpp \
    neslabusmainwidget.cpp \
    experimentsetup.cpp \
    stagesetup.cpp
