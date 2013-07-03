QT += svg
TARGET = toftune
TEMPLATE = lib
PROVIDER = MS-Cheminformatics

include(../../tofplugin.pri)
include(../../qwt.pri)
include(../../ace_tao.pri)
include(../../boost.pri)

DEFINES += TOFTUNE_LIBRARY
INCLUDEPATH += ../../libs ../../adplugins

SOURCES += toftuneplugin.cpp \
           toftunemode.cpp \
           mainwindow.cpp \
           sideframe.cpp \
           tofsignalmonitorview.cpp \
           monitorui.cpp \
           adpluginfactory.cpp \
           bezelwidget.cpp \
           ionsourcewidget.cpp \
           analyzerwidget.cpp \
           acquisitionwidget.cpp \
           datamediator.cpp \
           doublespinslider.cpp \
           spinslider.cpp \
           isequenceimpl.cpp \
           ieditorfactory.cpp
           
           
HEADERS += toftuneplugin.hpp\
           toftune_global.hpp\
           toftuneconstants.hpp \
           toftunemode.hpp \
           mainwindow.hpp \
           sideframe.hpp \
           tofsignalmonitorview.hpp \
           monitorui.hpp \
           adpluginfactory.hpp \
           receiver_i.hpp \
           bezelwidget.hpp \
           ionsourcewidget.hpp \
           analyzerwidget.hpp \
           acquisitionwidget.hpp \
           datamediator.hpp \
           interactor.hpp \
           doublespinslider.hpp \
           spinslider.hpp \
           isequenceimpl.hpp \
           ieditorfactory.hpp

OTHER_FILES = toftune.pluginspec

macx {
  LIBS += -lTAO -lTAO_Utils -lTAO_PI -lTAO_PortableServer -lTAO_AnyTypeCode -lACE -lACE
} else {
  LIBS += -l$$qtLibraryTarget( TAO ) \
    -l$$qtLibraryTarget( TAO_Utils ) \
    -l$$qtLibraryTarget( TAO_PI ) \
    -l$$qtLibraryTarget( TAO_PortableServer ) \
    -l$$qtLibraryTarget( TAO_AnyTypeCode ) \
    -l$$qtLibraryTarget( ACE )
}

LIBS += \
    -l$$qtLibraryTarget( Core ) \
    -l$$qtLibraryTarget( tofinterface ) \
    -l$$qtLibraryTarget( adextension ) \
    -l$$qtLibraryTarget( adinterface ) \
    -l$$qtLibraryTarget( adplugin ) \
    -l$$qtLibraryTarget( adorbmgr ) \
    -l$$qtLibraryTarget( adportable ) \
    -l$$qtLibraryTarget( adcontrols ) \
    -l$$qtLibraryTarget( acewrapper ) \
    -l$$qtLibraryTarget( adwplot ) \
    -l$$qtLibraryTarget( qtwrapper ) \
    -l$$qtLibraryTarget( qwt )


RESOURCES += \
    resources.qrc

FORMS += \
    monitorui.ui \
    bezelwidget.ui \
    ionsourcewidget.ui \
    analyzerwidget.ui \
    acquisitionwidget.ui
