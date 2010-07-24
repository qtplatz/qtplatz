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
    deviceproxy.cpp \
    devicetext.cpp

HEADERS  += maincontrollerwindow.h \
    eventreceiver.h \
    treemodel.h \
    treeitem.h \
    deviceproxy.h \
    devicetext.h

IDLFILES += controller.idl
#GENERATED_FILES = controllerC.cpp controllerS.cpp
SOURCES += controllerC.cpp controllerS.cpp

tao_idlC.input = IDLFILES
tao_idlC.output = ${QMAKE_FILE_BASE}C.h
tao_idlC.variable_out = GENERATED_FILES
tao_idlC.depends = ${QMAKE_FILE_IN}
tao_idlC.commands = tao_idl -Wb,pre_include=ace/pre.h -Wb,post_include=ace/post.h -I$$(TAO_ROOT) ${QMAKE_FILE_IN}
tao_idlC.name = TAO_IDL_C ${QMAKE_FILE_IN}
tao_idlC.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += tao_idlC

tao_idlS.input = IDLFILES
tao_idlS.output = ${QMAKE_FILE_BASE}S.h
tao_idlS.variable_out = GENERATED_FILES
tao_idlS.depends = ${QMAKE_FILE_IN}
tao_idlS.commands = tao_idl -Wb,pre_include=ace/pre.h -Wb,post_include=ace/post.h -I$$(TAO_ROOT) ${QMAKE_FILE_IN}
tao_idlS.name = TAO_IDL_S ${QMAKE_FILE_IN}
tao_idlC.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += tao_idlS

FORMS    += maincontrollerwindow.ui

RESOURCES += \
    controller.qrc
