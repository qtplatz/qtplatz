#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T17:59:52
#
#-------------------------------------------------

QT       += xml

TARGET = sequence
TEMPLATE = lib

PROVIDER = MS-Cheminformatics
include(../../qtplatzplugin.pri)
include(../../boost.pri)

LIBS += -L$$IDE_PLUGIN_PATH/QtProject
LIBS += -l$$qtLibraryTarget( Core )

# Link order caution: adportable should be linked after adfs for utf::to_utf8 reference

LIBS += -l$$qtLibraryTarget(adcontrols) \
	-l$$qtLibraryTarget(adutils) \
	-l$$qtLibraryTarget(adplot) \
	-l$$qtLibraryTarget(acewrapper) \
        -l$$qtLibraryTarget(qtwrapper) \
	-l$$qtLibraryTarget(xmlparser) \
        -l$$qtLibraryTarget(adplugin) \
	-l$$qtLibraryTarget(adextension) \
        -l$$qtLibraryTarget(adsequence) \
        -l$$qtLibraryTarget(adwidgets) \
        -l$$qtLibraryTarget(adlog) \
        -l$$qtLibraryTarget(adfs) \
	-l$$qtLibraryTarget(adportable)

!win32 {
  LIBS += -lboost_system -lboost_filesystem -lboost_serialization -lboost_date_time -ldl
}

DEFINES += SEQUENCE_LIBRARY

SOURCES +=  sequenceplugin.cpp \
    sequenceeditor.cpp \
    sequenceeditorfactory.cpp \
    sequencefile.cpp \
    mainwindow.cpp \
    mode.cpp \
    sequencewnd.cpp \
    sequencedelegate.cpp \
    sequenceview.cpp

HEADERS += sequence_global.h \
    sequencefile.hpp \
    sequenceplugin.hpp \
    sequenceeditorfactory.hpp \
    sequenceeditor.hpp \
    constants.hpp \
    mainwindow.hpp \
    mode.hpp \
    sequencewnd.hpp \
    sequencedelegate.hpp \
    sequenceview.hpp

OTHER_FILES += sequence.pluginspec \
    sequence-mimetype.xml \
    sequence_dependencies.pri

RESOURCES += \
    sequence.qrc

FORMS += \
    sequencewidget.ui
