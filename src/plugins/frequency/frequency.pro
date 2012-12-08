TARGET = frequency
TEMPLATE = lib

DEFINES += FREQUENCY_LIBRARY

# frequency files

SOURCES += frequencyplugin.cpp \
    mode.cpp \
    mainwindow.cpp \
    waveformview.cpp

HEADERS += frequencyplugin.hpp \
        frequency_global.hpp \
        frequencyconstants.hpp \
    mode.hpp \
    constants.hpp \
    mainwindow.hpp \
    waveformview.hpp

PROVIDER = ScienceLiaison
include(../../qtplatz_plugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)

include(../../boost.pri)
include(../../qwt.pri)

LIBS += -L$$IDE_PLUGIN_PATH/Nokia -L$$IDE_LIBRARY_PATH

FORMS += \
    waveformview.ui

RESOURCES += \
    frequency.qrc

