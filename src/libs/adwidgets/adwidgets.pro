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

!win32 {
  LIBS += -lboost_filesystem -lboost_system 
}

DEFINES += ADWIDGETS_LIBRARY

SOURCES += adwidgets.cpp \
        targetingform.cpp \
        targetingwidget.cpp \
        tableview.cpp \
        targetingtable.cpp \
        peptidewidget.cpp \
        peptidetable.cpp \
        delegatehelper.cpp \
        controlmethodtable.cpp \
        controlmethodwidget.cpp \
        centroidform.cpp \
        mspeaktable.cpp \
        msquantable.cpp

HEADERS += adwidgets.hpp\
        adwidgets_global.hpp \
        targetingform.hpp \
        spin_t.hpp \
        targetingwidget.hpp \
        tableview.hpp \
        targetingtable.hpp \
        peptidewidget.hpp \
        peptidetable.hpp \
        delegatehelper.hpp \
        controlmethodtable.hpp \
        controlmethodwidget.hpp \
        centroidform.hpp \
        mspeaktable.hpp \
        msquantable.hpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}

FORMS += \
    targetingform.ui \
    centroidform.ui
