#-------------------------------------------------
#
# Project created by QtCreator 2013-01-03T08:43:54
#
#-------------------------------------------------

QT       -= core gui

TARGET = adsequence
TEMPLATE = lib

include(../../qtplatz_library.pri)
include(../../boost.pri)

!win32 {
  LIBS += -lboost_date_time -lboost_system -lboost_wserialization -lboost_serialization 
}

#LIBS += -L$$IDE_LIBRARY_PATH
LIBS += -l$$qtLibraryTarget( adportable )


DEFINES += ADSEQUENCE_LIBRARY

SOURCES += adsequence.cpp \
    schema.cpp \
    sequence.cpp

HEADERS += adsequence.hpp\
        adsequence_global.hpp \
    schema.hpp \
    sequence.hpp

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xE846F3CA
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = adsequence.dll
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
