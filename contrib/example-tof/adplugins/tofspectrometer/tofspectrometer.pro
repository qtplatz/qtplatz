#-------------------------------------------------
#
# Project created by QtCreator 2011-07-05T12:51:49
#
#-------------------------------------------------

QT       -= core gui

TARGET = tofspectrometer
TEMPLATE = lib
DEFINES += TOFSPECTROMETER_LIBRARY
INCLUDEPATH += ../../libs

include(../../tofadplugin.pri)
include(../../boost.pri)
include(../../ace_tao.pri)

LIBS += -l$$qtLibraryTarget( adinterface ) \
        -l$$qtLibraryTarget( tofinterface ) \
        -l$$qtLibraryTarget( adportable ) \
        -l$$qtLibraryTarget( adplugin ) \
        -l$$qtLibraryTarget( adcontrols )

!win32: LIBS += -lboost_serialization -lboost_wserialization -lboost_date_time

LIBS += -l$$qtLibraryTarget( ACE )

SOURCES += tofspectrometer.cpp \
           tof.cpp \
           tofinterpreter.cpp \
           tofscanlaw.cpp

HEADERS += tofspectrometer.hpp\
           tofspectrometer_global.hpp \
           tof.hpp \
           tofinterpreter.hpp \
           tofscanlaw.hpp \
           constants.hpp

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE2E0BEE3
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = tofspectrometer.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
