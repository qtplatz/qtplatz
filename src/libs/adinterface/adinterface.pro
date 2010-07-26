#-------------------------------------------------
#
# Project created by QtCreator 2010-07-25T06:44:13
#
#-------------------------------------------------

QT       -= core gui

TARGET = interface
TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$(ACE_ROOT) $$(TAO_ROOT) ../libs
LIBS *= -L$$(ACE_ROOT)/lib

SOURCES += interface.cpp

HEADERS += interface.h

IDLFILES += \
	controlmethod.idl \
	controlserver.idl \
	global_constants.idl \
	receiver.idl \
	samplebroker.idl \
	signalobserver.idl

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

GENERATED_FILES += global_constantsC.cpp \
	controlserverC.cpp \
	controlserverS.cpp \
	controlmethodC.cpp \ 
	controlmethodS.cpp \ 
	controlmethodC.cpp \
	controlmethodS.cpp \
	controlserverC.cpp \
	controlserverS.cpp \
	global_constantsC.cpp \
	global_constantsS.cpp \
	receiverC.cpp \
	receiverS.cpp \
	samplebrokerC.cpp \
	signalobserverS.cpp
