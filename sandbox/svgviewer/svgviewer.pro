#-------------------------------------------------
#
# Project created by QtCreator 2012-10-17T12:03:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = svgviewer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    svgview.cpp

HEADERS  += mainwindow.hpp \
    svgview.hpp

FORMS    += mainwindow.ui
