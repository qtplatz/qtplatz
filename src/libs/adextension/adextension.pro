#-------------------------------------------------
#
# Project created by QtCreator 2013-01-04T16:30:19
#
#-------------------------------------------------

QT       -= gui

TARGET = adextension
TEMPLATE = lib

include(../../qtplatzlibrary.pri)
include(../../ace_tao.pri)

DEFINES += ADEXTENSION_LIBRARY

SOURCES += adextension.cpp \
           isequence.cpp \
           ieditorfactory.cpp \
           iobjectref.cpp

HEADERS += adextension.hpp\
           adextension_global.hpp \
           isequence.hpp \
           ieditorfactory.hpp \
           iobjectref.hpp

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
