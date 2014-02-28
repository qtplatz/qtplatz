# U5303A plugin qmake 
lessThan( QT_MAJOR_VERSION, 5 ): error("Qt5 or later version required")

QT += core svg printsupport widgets

PROVIDER = MS-Cheminformatics
DEFINES += U5303A_LIBRARY

include(../../agilentplugin.pri)
include(../../qwt.pri)
include(../../boost.pri)

INCLUDEPATH *= $$OUT_PWD/../../libs

# u5303 files

SOURCES += u5303aplugin.cpp \
           mainwindow.cpp \
           u5303amode.cpp \
           document.cpp \
           waveformwnd.cpp \
    u5303aform.cpp \
    u5303amethodtable.cpp \
    u5303amethodwidget.cpp

HEADERS += u5303aplugin.hpp \
        u5303a_global.hpp \
        u5303a_constants.hpp \
        mainwindow.hpp \
        u5303amode.hpp \
        document.hpp \
        waveformwnd.hpp \
    u5303aform.hpp \
    u5303amethodtable.hpp \
    u5303amethodwidget.hpp

LIBS += -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adlog) \
        -l$$qtLibraryTarget(adfs) \
        -l$$qtLibraryTarget(adextension) \
        -l$$qtLibraryTarget(adwplot) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(qtwrapper) \
        -l$$qtLibraryTarget(u5303a)

# Qt Creator linking

## set the QTC_SOURCE environment variable to override the setting here
#QTCREATOR_SOURCES = $$(QTC_SOURCE)

## set the QTC_BUILD environment variable to override the setting here
#IDE_BUILD_TREE = $$(QTC_BUILD)

QTC_PLUGIN_NAME = u5303aplugin
QTC_LIB_DEPENDS += \
    # nothing here at this time

QTC_PLUGIN_DEPENDS += \
    coreplugin

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

###### End _dependencies.pri contents ######

RESOURCES += \
    u5303a.qrc

FORMS += \
    u5303aform.ui
