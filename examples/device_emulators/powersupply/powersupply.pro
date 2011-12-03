#-------------------------------------------------
#
# Project created by QtCreator 2011-12-03T11:47:40
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = powersupply
CONFIG   += console
CONFIG   -= app_bundle
LIBS *= -lboost_system -lboost_date_time
TEMPLATE = app


SOURCES += main.cpp
