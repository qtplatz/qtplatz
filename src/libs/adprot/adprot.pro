#-------------------------------------------------
#
# Project created by QtCreator 2014-02-08T07:46:36
#
#-------------------------------------------------

QT       -= core gui

TARGET = adprot
TEMPLATE = lib
INCLUDEPATH += ../../libs
include(../../qtplatzlibrary.pri)
include(../../boost.pri)

DEFINES += ADPROT_LIBRARY

SOURCES += adprot.cpp \
    peptide.cpp \
    peptides.cpp \
    protease.cpp \
    protein.cpp \
    sequence.cpp \
    protfile.cpp \
    aminoacid.cpp

HEADERS += adprot.hpp\
        adpeptide_global.hpp \
    peptide.hpp \
    peptides.hpp \
    protease.hpp \
    protein.hpp \
    sequence.hpp \
    protfile.hpp \
    aminoacid.hpp

!win32 {
  LIBS += -lboost_system
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
