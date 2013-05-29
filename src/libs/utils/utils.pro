TEMPLATE = lib
TARGET = Utils
QT += gui \
    network

DEFINES += QTCREATOR_UTILS_LIB
include(../../qtplatzlibrary.pri)

SOURCES += \
    appmainwindow.cpp \
    pathchooser.cpp \
    filewizardpage.cpp \
    filewizarddialog.cpp \
    basevalidatinglineedit.cpp \
    filenamevalidatinglineedit.cpp \
    qtcolorbutton.cpp \
    treewidgetcolumnstretcher.cpp \
    styledbar.cpp \
    stylehelper.cpp \
    fancymainwindow.cpp

win32 { 
    SOURCES += \
        winutils.cpp
    HEADERS += winutils.h
}

HEADERS += utils_global.h \
    appmainwindow.h \
    listutils.h \
    pathchooser.h \
    filewizardpage.h \
    filewizarddialog.h \
    basevalidatinglineedit.h \
    filenamevalidatinglineedit.h \
    qtcolorbutton.h \
    treewidgetcolumnstretcher.h \
    qtcassert.h \
    styledbar.h \
    stylehelper.h \
    fancymainwindow.h

FORMS += filewizardpage.ui \
    projectintropage.ui \
    newclasswidget.ui \
    submiteditorwidget.ui \
    checkablemessagebox.ui
RESOURCES += utils.qrc
