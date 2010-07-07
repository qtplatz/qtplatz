#-------------------------------------------------
#
# Project created by QtCreator 2010-07-07T14:07:09
#
#-------------------------------------------------

include(../../qtPlatz.pri)
QT       += core gui

TARGET = test_controller
TEMPLATE = app
include(../boost.pri)

INCLUDEPATH += $$(ACE_ROOT) ../libs

Debug {
    LIBS += -l$$qtLibraryTarget(acewrapperd)
    LIBS += -L$$(ACE_ROOT)/lib -lACEd
}
Release {
    LIBS += -l$$qtLibraryTarget(acewrapper)
    LIBS += -L$$(ACE_ROOT)/lib -lACE
}

SOURCES += main.cpp\
        maincontrollerwindow.cpp \
    mcastserver.cpp

HEADERS  += maincontrollerwindow.h \
    mcastserver.h \
    Callback.h

FORMS    += maincontrollerwindow.ui
