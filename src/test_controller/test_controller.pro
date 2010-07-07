#-------------------------------------------------
#
# Project created by QtCreator 2010-07-07T14:07:09
#
#-------------------------------------------------

QT       += core gui

TARGET = test_controller
TEMPLATE = app
include(../boost.pri)

INCLUDEPATH += $$(ACE_ROOT) ../libs

debug {
    LIBS += -L$$(ACE_ROOT)/lib -lACEd
    LIBS += -L../../lib/qtPlatz -lacewrapperd
}
release {
    LIBS += -L$$(ACE_ROOT)/lib -lACE
    LIBS += -L../../lib/qtPlatz -lacewrapper
}

SOURCES += main.cpp\
        maincontrollerwindow.cpp \
    mcastserver.cpp

HEADERS  += maincontrollerwindow.h \
    mcastserver.h \
    Callback.h

FORMS    += maincontrollerwindow.ui
