#-------------------------------------------------
#
# Project created by QtCreator 2014-02-15T13:47:49
#
#-------------------------------------------------

QT       += widgets svg

TARGET = adwidgets
TEMPLATE = lib

include(../../qtplatzlibrary.pri)
include(../../boost.pri)

LIBS += -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adprot) \
        -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(qtwrapper)

DEFINES += ADWIDGETS_LIBRARY

SOURCES += adwidgets.cpp \
    targetingform.cpp \
    targetingwidget.cpp \
    tableview.cpp \
    targetingtable.cpp \
    peptidewidget.cpp \
    peptidetable.cpp \
    delegatehelper.cpp

HEADERS += adwidgets.hpp\
        adwidgets_global.hpp \
    targetingform.hpp \
    spin_t.hpp \
    targetingwidget.hpp \
    tableview.hpp \
    targetingtable.hpp \
    peptidewidget.hpp \
    peptidetable.hpp \
    delegatehelper.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    targetingform.ui
