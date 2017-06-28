#-------------------------------------------------
#
# Project created by QtCreator 2017-01-12T08:55:18
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GroundStation_V3
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp \
    incrementalplot.cpp \
    plot.cpp \
    scrollzoomer.cpp \
    scrollbar.cpp

HEADERS  += mainwindow.h \
    settingsdialog.h \
    console.h \
    incrementalplot.h \
    plot.h \
    scrollzoomer.h \
    scrollbar.h

FORMS    += mainwindow.ui \
    settingsdialog.ui



unix:!macx: LIBS += -L$$PWD/../../../../../usr/local/qwt-6.1.3/lib/ -lqwt

INCLUDEPATH += $$PWD/../../../../../usr/local/qwt-6.1.3/include
DEPENDPATH += $$PWD/../../../../../usr/local/qwt-6.1.3/include
