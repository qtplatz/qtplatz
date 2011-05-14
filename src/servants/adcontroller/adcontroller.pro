#-------------------------------------------------
#
# Project created by QtCreator 2010-07-30T12:33:13
#
#-------------------------------------------------

QT       -= core gui

TARGET = adcontroller
TEMPLATE = lib

include(../../qtplatz_servant.pri)

include(../../libs/acewrapper/acewrapper_dependencies.pri)
include(../../libs/adinterface/adinterface_dependencies.pri)
include(../../boost.pri)
LIBS *= -l$$qtLibraryTarget(adinterface)
INCLUDEPATH *= $$OUT_PWD/../../libs

DEFINES += ADCONTROLLER_LIBRARY

SOURCES += adcontroller.cpp \
        ibroker.cpp \
        ibrokermanager.cpp \
        iproxy.cpp \
        manager_i.cpp \
        mcast_handler.cpp \
        message.cpp \
        observer_i.cpp \
        oproxy.cpp \
        receiver_i.cpp \
        session_i.cpp \
        signal_handler.cpp \
    cache.cpp

HEADERS += adcontroller.hpp \
        adcontroller_global.h \
        constants.hpp \
        ibroker.hpp \
        ibrokermanager.hpp \
        iproxy.hpp \
        manager_i.hpp \
        marshal.hpp \
        mcast_handler.hpp \
        message.hpp \
        observer_i.hpp \
        oproxy.hpp \
        receiver_i.hpp \
        session_i.hpp \
        signal_handler.hpp \
    cache.hpp
