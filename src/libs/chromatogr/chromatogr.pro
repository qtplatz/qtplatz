#-------------------------------------------------
#
# Project created by QtCreator 2012-05-02T10:35:56
#
#-------------------------------------------------

QT       -= core gui

TARGET = chromatogr
TEMPLATE = lib

DEFINES += CHROMATOGR_LIBRARY

SOURCES += chromatogr.cpp

HEADERS += chromatogr.hpp\
        chromatogr_global.hpp

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEB5FDD68
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = chromatogr.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
