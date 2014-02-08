#-------------------------------------------------
#
# Project created by QtCreator 2014-02-08T07:46:36
#
#-------------------------------------------------

QT       -= core gui

TARGET = adpeptide
TEMPLATE = lib
INCLUDEPATH += ../../libs
include(../../qtplatzlibrary.pri)
include(../../boost.pri)

DEFINES += ADPEPTIDE_LIBRARY

SOURCES += adpeptide.cpp \
    peptide.cpp \
    peptides.cpp \
    protease.cpp \
    protein.cpp \
    sequence.cpp \
    protfile.cpp \
    aminoacid.cpp

HEADERS += adpeptide.hpp\
        adpeptide_global.hpp \
    peptide.hpp \
    peptides.hpp \
    protease.hpp \
    protein.hpp \
    sequence.hpp \
    protfile.hpp \
    aminoacid.hpp

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
