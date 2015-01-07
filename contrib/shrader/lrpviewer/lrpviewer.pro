#-------------------------------------------------
#
# Project created by QtCreator 2014-04-03T07:52:02
#
#-------------------------------------------------

QT       += core gui svg printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtspcview
TEMPLATE = app

include(../galactic.pri)
include(../../../src/qwt.pri)
include(../../../src/boost.pri)

LIBS += -L../../../lib/qtplatz \
        -L../../../lib/qtplatz/plugins/MS-Cheminformatics \
        -l$$qtLibraryTarget(adplot) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(qtwrapper) \
        -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(spcfile)

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.hpp

FORMS    +=

CONFIG(release, debug|release): DESTDIR = $$OUT_PWD/release
CONFIG(debug, debug|release): DESTDIR = $$OUT_PWD/debug

DllFiles.path = $$DESTDIR
DllFiles.files = ../../../lib/qtplatz/$$qtLibraryTarget(adcontrols).dll \
                 ../../../lib/qtplatz/$$qtLibraryTarget(spcfile).dll \

INSTALLS += DllFiles
