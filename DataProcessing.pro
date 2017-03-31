#-------------------------------------------------
#
# Project created by QtCreator 2017-02-07T19:42:51
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DataProcessing
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    parsercsv.cpp \
    processingunit.cpp

HEADERS  += mainwindow.h \
    parsercsv.h \
    processingunit.h

FORMS    += mainwindow.ui
