#-------------------------------------------------
#
# Project created by QtCreator 2010-07-07T14:08:12
#
#-------------------------------------------------

QT       += core gui xml

TARGET = controller
TEMPLATE = app
include(../boost.pri)
include(../adilibrary.pri)

INCLUDEPATH += $$(ACE_ROOT) $$(TAO_ROOT) ../libs
LIBS *= -L$$IDE_LIBRARY_PATH -L$$(ACE_ROOT)/lib
Debug {
    LIBS += -ladportabled -lacewrapperd -lACEd
    CONFIG += debug
}
Release {
    LIBS += -ladportable -lacewrapper -lACE
}

SOURCES += main.cpp\
    maincontrollerwindow.cpp \
    main.cpp \
    eventreceiver.cpp \
    treemodel.cpp \
    treeitem.cpp \
    deviceproxy.cpp

HEADERS  += maincontrollerwindow.h \
    eventreceiver.h \
    treemodel.h \
    treeitem.h \
    deviceproxy.h

IDLFILES += controller.idl

idl_tao.output = ${QMAKE_FILE_BASE}S.cpp
idl_tao.output += ${QMAKE_FILE_BASE}C.cpp
idl_tao.input = IDLFILES
idl_tao.variable_out=SOURCES
idl_tao.commands = tao_idl -Wb,pre_include=ace/pre.h -Wb,post_include=arc/post.h -I$$(TAO_ROOT) ${QMAKE_FILE_IN}
idl_tao.name = tao_idl ${QMAKE_FILE_IN}

QMAKE_EXTRA_COMPILERS += idl_tao


#PRE_TARGETDEPS += $$idl.output
#SOURCES += controllerC.cpp controllerS.cpp
#QMAKE_EXTRA_WIN_TARGETS += controllerC.cpp controllerS.cpp

FORMS    += maincontrollerwindow.ui

RESOURCES += \
    controller.qrc

OTHER_FILES += \
    controller.idl
