#-------------------------------------------------
#
# Project created by QtCreator 2010-08-14T07:16:37
#
#-------------------------------------------------

QT += core gui

TARGET = qtwidgets2
TEMPLATE = lib

PROVIDER = MS-Cheminformatics
include(../../adplugin.pri)
include(../../boost.pri)
include( ../../ace_tao.pri )

LIBS += -l$$qtLibraryTarget(adportable) -l$$qtLibraryTarget(adcontrols) \
    -l$$qtLibraryTarget(adplugin) -l$$qtLibraryTarget(qtwrapper) \
    -l$$qtLibraryTarget(xmlparser)

!win32 {
  LIBS *= -lboost_filesystem -lboost_system
}

INCLUDEPATH *= $(ACE_ROOT) $(TAO_ROOT)

DEFINES += QTWIDGETS2_LIBRARY

SOURCES += factory.cpp \
        mscalibratedelegate.cpp \
        mscalibrationform.cpp \
        mscalibsummarydelegate.cpp \
        mscalibsummarywidget.cpp \
        standarditemhelper.cpp \
        qtwidgets2.cpp

HEADERS += \
        factory.hpp \
        mscalibratedelegate.hpp \
        mscalibrationform.hpp \
        mscalibsummarydelegate.hpp \
        mscalibsummarywidget.hpp \
        standarditemhelper.hpp \
        qtwidgets2.hpp

FORMS += \
    mscalibrationform.ui

RESOURCES +=

