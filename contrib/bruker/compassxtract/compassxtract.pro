#-------------------------------------------------
#
# Project created by QtCreator 2012-04-21T09:34:34
#
#-------------------------------------------------

QT       -= core gui

include(../../contrib.pri)

#TARGET = compassxtract
TARGET = $$qtLibraryTarget($$TARGET)
TEMPLATE = lib

INCLUDEPATH += .
INCLUDEPATH += $$QTPLATZ_SOURCE_TREE/src
INCLUDEPATH += \"C:/Program Files (x86)/Bruker Daltonik/CompassXtract\"

DESTDIR = $$QTPLATZ_PLUGIN_PATH/$$PROVIDER
DEFINES += COMPASSXTRACT_LIBRARY

SOURCES += compassxtract.cpp \
    datafile_factory.cpp \
    datafile.cpp \
    safearray.cpp

HEADERS += compassxtract.hpp\
        compassxtract_global.hpp \
    datafile_factory.hpp \
    datafile.hpp \
    safearray.hpp

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEF194CC6
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = compassxtract.dll
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
