#-------------------------------------------------
#
# Project created by QtCreator 2010-08-14T07:16:37
#
#-------------------------------------------------

QT += core gui svg printsupport

TARGET = qtwidgets2
TEMPLATE = lib

PROVIDER = MS-Cheminformatics
include(../../adplugin.pri)
include(../../boost.pri)
include( ../../ace_tao.pri )

LIBS += -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adextension) \
        -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adlog) \
        -l$$qtLibraryTarget(qtwrapper) \
        -l$$qtLibraryTarget(xmlparser)

!win32 {
  LIBS *= -lboost_filesystem -lboost_system
}

INCLUDEPATH *= $(ACE_ROOT) $(TAO_ROOT)

#workaround for 5.1.0 cause a fatal error on qitemdelegate.h can not find qabstractitemdelegate.h
greaterThan( QT_MAJOR_VERSION, 4 ): INCLUDEPATH += ${QTDIR}/include  

DEFINES += QTWIDGETS2_LIBRARY

SOURCES += factory.cpp \
        mscalibratedelegate.cpp \
        mscalibrationform.cpp \
        mscalibsummarydelegate.cpp \
        mscalibsummarywidget.cpp \
        mspeakview.cpp \
        mspeaksummary.cpp \
        standarditemhelper.cpp \
        tableview.cpp \
        toftable.cpp \
        qtwidgets2.cpp \
        mschromatogramwidget.cpp

HEADERS += \
        factory.hpp \
        mscalibratedelegate.hpp \
        mscalibrationform.hpp \
        mspeakview.hpp \
        mspeaksummary.hpp \
        mscalibsummarydelegate.hpp \
        mscalibsummarywidget.hpp \
        standarditemhelper.hpp \
        tableview.hpp \
        toftable.hpp \
        qtwidgets2.hpp \
        mschromatogramwidget.hpp

FORMS += \
    mscalibrationform.ui

RESOURCES +=

