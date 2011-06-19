TEMPLATE = lib
TARGET = Welcome
QT += declarative
include(../../qtplatz_plugin.pri)
include(welcome_dependencies.pri)
HEADERS += welcomeplugin.h \
    welcomemode.h \
    welcome_global.h
SOURCES += welcomeplugin.cpp \
    welcomemode.cpp

FORMS += welcomemode.ui
RESOURCES += welcome.qrc
DEFINES += WELCOME_LIBRARY
OTHER_FILES += Welcome.pluginspec \
    qml/webbrowser.qml
