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

#HEADERS += controllerC.h controllerS.h
#SOURCES += controllerC.cpp controllerS.cpp

IDLFILES += controller.idl
GENERATED_FILES = controllerC.cpp controllerS.cpp

tao_idl.input = IDLFILES
tao_idl.output = ${QMAKE_FILE_BASE}C.cpp
isEmpty(vcproj):tao_idl.variable_out = PRE_TARGETDEPS
tao_idl.depends = ${QMAKE_FILE_IN}
tao_idl.commands = tao_idl -Wb,pre_include=ace/pre.h -Wb,post_include=ace/post.h -I$$(TAO_ROOT) ${QMAKE_FILE_IN}
tao_idl.name = tao_idl ${QMAKE_FILE_IN}
tao_idl.CONFIG += link
QMAKE_EXTRA_COMPILERS += tao_idl

tao_idl_s.input = IDLFILES
tao_idl_s.output = ${QMAKE_FILE_BASE}S.cpp
isEmpty(vcproj):tao_idl_s.variable_out = PRE_TARGETDEPS
tao_idl_s.depends = ${QMAKE_FILE_IN}
tao_idl_s.commands = tao_idl -Wb,pre_include=ace/pre.h -Wb,post_include=ace/post.h -I$$(TAO_ROOT) ${QMAKE_FILE_IN}
tao_idl_s.name = tao_idl ${QMAKE_FILE_IN}
tao_idl_s.CONFIG += link
QMAKE_EXTRA_COMPILERS += tao_idl_s

FORMS    += maincontrollerwindow.ui

RESOURCES += \
    controller.qrc

OTHER_FILES += \
    controller.idl
