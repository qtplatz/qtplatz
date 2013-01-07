#-------------------------------------------------
#
# Project created by QtCreator 2010-06-19T17:59:52
#
#-------------------------------------------------

QT       += xml

TARGET = sequence
TEMPLATE = lib

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)
include(sequence_dependencies.pri)

LIBS += -L$$IDE_PLUGIN_PATH/Nokia
include(../../plugins/coreplugin/coreplugin.pri)

LIBS += -l$$qtLibraryTarget(adcontroller) -l$$qtLibraryTarget(adcontrols) \
	-l$$qtLibraryTarget(adutils)      -l$$qtLibraryTarget(adinterface) \
	-l$$qtLibraryTarget(adportable)   -l$$qtLibraryTarget(adwplot) \
	-l$$qtLibraryTarget(acewrapper)   -l$$qtLibraryTarget(qtwrapper) \
	-l$$qtLibraryTarget(xmlparser)    -l$$qtLibraryTarget(adplugin) \
	-l$$qtLibraryTarget(adextension)  -l$$qtLibraryTarget(adsequence)

LIBS += -l$$qtLibraryTarget( TAO ) \
        -l$$qtLibraryTarget( TAO_Utils ) \
        -l$$qtLibraryTarget( TAO_PI ) \
        -l$$qtLibraryTarget( TAO_PortableServer ) \
        -l$$qtLibraryTarget( TAO_AnyTypeCode ) \
        -l$$qtLibraryTarget( ACE )


DEFINES += SEQUENCE_LIBRARY

SOURCES +=  sequenceplugin.cpp \
    sequenceeditor.cpp \
    sequenceeditorfactory.cpp \
    sequence.cpp \
    mainwindow.cpp \
    mode.cpp \
    sequencewidget.cpp \
    sequencedelegate.cpp \
    sequenceview.cpp

HEADERS += sequence_global.h \
    sequence.hpp \
    sequenceplugin.hpp \
    sequenceeditorfactory.hpp \
    sequenceeditor.hpp \
    constants.hpp \
    mainwindow.hpp \
    mode.hpp \
    sequencewidget.hpp \
    sequencedelegate.hpp \
    sequenceview.hpp

OTHER_FILES += sequence.pluginspec \
    sequence-mimetype.xml \
    sequence_dependencies.pri

RESOURCES += \
    sequence.qrc

FORMS += \
    sequencewidget.ui
