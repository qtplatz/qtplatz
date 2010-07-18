#-------------------------------------------------
#
# Project created by QtCreator 2010-07-07T14:08:12
#
#-------------------------------------------------

QT       += core gui xml

TARGET = controller
TEMPLATE = app
include(../boost.pri)
include(../adilibrary.pri)

INCLUDEPATH += $$(ACE_ROOT) ../libs
LIBS *= -L$$IDE_LIBRARY_PATH -L$$(ACE_ROOT)/lib
Debug {
    LIBS += -ladportabled -lacewrapperd -lACEd
    CONFIG += debug
}
Release {
    LIBS += -ladportable -lacewrapper -lACE
}

SOURCES += main.cpp\
    maincontrollerwindow.cpp \
    main.cpp \
    eventreceiver.cpp \
    treemodel.cpp \
    treeitem.cpp \
    deviceproxy.cpp

HEADERS  += maincontrollerwindow.h \
    eventreceiver.h \
    treemodel.h \
    treeitem.h \
    deviceproxy.h

FORMS    += maincontrollerwindow.ui

RESOURCES += \
    controller.qrc
