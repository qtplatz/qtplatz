#-------------------------------------------------
#
# Project created by QtCreator 2013-01-04T16:30:19
#
#-------------------------------------------------

QT       -= gui

TARGET = adextension
TEMPLATE = lib

include(../../qtplatzlibrary.pri)

DEFINES += ADEXTENSION_LIBRARY

SOURCES += adextension.cpp \
           isequence.cpp \
           ieditorfactory.cpp \
           imonitorfactory.cpp \
           ipeptide.cpp \
           ipeptidehandler.cpp \
           ilogger.cpp \
    ibroker.cpp

HEADERS += adextension.hpp\
           adextension_global.hpp \
           isnapshothandler.hpp \
           isequence.hpp \
           ieditorfactory.hpp \
           imonitorfactory.hpp \
           ipeptide.hpp \
           ipeptidehandler.hpp \
           ilogger.hpp \
    ibroker.hpp

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
