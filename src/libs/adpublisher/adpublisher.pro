#-------------------------------------------------
#
# Project created by QtCreator 2014-08-12T18:01:52
#
#-------------------------------------------------

QT       += sql svg xml printsupport

#QT       -= gui

TARGET = adpublisher
TEMPLATE = lib

INCLUDEPATH += ../../libs
include(../../boost.pri)
include(../../qtplatzlibrary.pri)

DEFINES += ADPUBLISHER_LIBRARY

SOURCES += adpublisher.cpp \
           document.cpp \
           doceditor.cpp \
           doctree.cpp \
           doctext.cpp

HEADERS += adpublisher.hpp\
           adpublisher_global.hpp \
           document.hpp \
           doceditor.hpp \
           doctree.hpp \
           doctext.hpp

LIBS *= -L$$IDE_LIBRARY_PATH 
LIBS += -l$$qtLibraryTarget(xmlparser) -l$$qtLibraryTarget(adportable)

!win32: LIBS += -lboost_date_time -lboost_system

unix {
    target.path = /usr/lib
    INSTALLS += target
}

RESOURCES += \
    adpublisher.qrc

XSLT_FILES += xslt/debug.xslt
XSLT_DIR = xslt

xcopy2file.output = $$DESTDIR/plugins/MS-Cheminformatics/xslt/$${QMAKE_FUNC_FILE_IN_stripSrcDir}
xcopy2file.input += XSLT_DIR
isEmpty(vcproj):xcopy2file.variable_out = PRE_TARGETDEPS
xcopy2file.commands = $(COPY_DIR) ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
xcopy2file.name = XCOPY2FILE ${QMAKE_FILE_IN}
xcopy2file.CONFIG += no_link
#QMAKE_EXTRA_COMPILERS += xcopy2file
