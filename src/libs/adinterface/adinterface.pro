#-------------------------------------------------
#
# Project created by QtCreator 2010-07-25T06:44:13
#
#-------------------------------------------------

QT       -= core gui

TARGET = adinterface
TEMPLATE = lib
CONFIG += staticlib
include(../../qtplatz_library.pri)
include(../../boost.pri)
include(../../ace_tao.pri)
include(adinterface_dependencies.pri)

IDLFILES += \
    brokerevent.idl \
    controlmethod.idl \
    controlserver.idl \
    eventlog.idl \
    global_constants.idl \
    instrument.idl \
    loghandler.idl \
    receiver.idl \
    samplebroker.idl \
    signalobserver.idl \
    broker.idl

SOURCES += interface.cpp \
        eventlog_helper.cpp \
    	controlmethodhelper.cpp

HEADERS += interface.hpp \
        eventlog_helper.hpp \
        controlmethodhelper.hpp

for(idl, IDLFILES): PRE_TARGETDEPS += $$replace( idl, ".idl", "C.cpp" )

TAO_IDL = tao_idl

tao_idlC.name = TAO_IDL_C ${QMAKE_FILE_IN}
tao_idlC.input = IDLFILES
tao_idlC.output = ${QMAKE_FILE_BASE}C.cpp
tao_idlC.clean = ${QMAKE_FILE_BASE}C.cpp ${QMAKE_FILE_BASE}C.h ${QMAKE_FILE_BASE}C.inl
tao_idlC.commands = $${TAO_IDL} -Wb,pre_include=ace/pre.h -Wb,post_include=ace/post.h -I$$(TAO_ROOT) -I$${PWD} -I. ${QMAKE_FILE_IN}
tao_idlC.CONFIG += no_link
tao_idlC.depends = ${QMAKE_FILE_IN}
#isEmpty(vcproj): tao_idlC.variable_out = SOURCES
tao_idlC.variable_out = SOURCES
QMAKE_EXTRA_COMPILERS += tao_idlC

## following is the workaournd for qmake, which does not support multiple output on qmake_extra_compiler

tao_idlS.name = TAO_IDL_S ${QMAKE_FILE_IN}
tao_idlS.input = IDLFILES
tao_idlS.output = ${QMAKE_FILE_BASE}S.cpp
tao_idlS.clean = ${QMAKE_FILE_BASE}S.cpp ${QMAKE_FILE_BASE}S.h ${QMAKE_FILE_BASE}S.inl
tao_idlS.commands = @echo ===> tao_idl ${QMAKE_FILE_IN}
tao_idlS.CONFIG += no_link
tao_idlS.depends = ${QMAKE_FILE_IN}
#isEmpty(vcproj): tao_idlS.variable_out = SOURCES
tao_idlS.variable_out = SOURCES
QMAKE_EXTRA_COMPILERS += tao_idlS

OTHER_FILES += \
    adinterface_dependencies.pri

!isEmpty(vcproj) {
  for(idl, IDLFILES): OBJECTIVE_SOURCES += $$replace( idl, ".idl", "C.cpp" )
  for(idl, IDLFILES): OBJECTIVE_SOURCES += $$replace( idl, ".idl", "S.cpp" )
}

