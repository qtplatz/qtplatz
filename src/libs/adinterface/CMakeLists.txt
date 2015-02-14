# adinterface
cmake_minimum_required (VERSION 2.8.11)
project (adinterface)

include( TaoIDL )

set(TAO_IDL tao_idl)
set(IDL_INCLUDES "-I${QTPLATZ_SOURCE_DIR}/src/libs")
set(IDL_FLAGS "-Wb,pre_include=workaround/ace/pre.h -Wb,post_include=workaround/ace/post.h")

find_package( ace+tao )
if ( ace+tao_FOUND )
  include_directories(
    ${ACE+TAO_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/src/libs
    )
endif()

set( CMAKE_INCLUDE_CURRENT_DIR ON )

set( IDLFILES
  brokerevent.idl
  controlmethod.idl
  controlserver.idl
  eventlog.idl
  global_constants.idl
  instrument.idl
  loghandler.idl
  receiver.idl
  samplebroker.idl
  signalobserver.idl
  broker.idl
  brokerclient.idl
  )

if ( MSVC )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
elseif( APPLE )
  add_definitions( -Wno-deprecated-register )
endif()

ADD_CORBA_SERVERS(GENERATED_CPP GENERATED_H ${IDLFILES})
SOURCE_GROUP("GENERATED" FILES ${GENERATED_CPP})

add_library( adinterface  
  interface.cpp
  eventlog_helper.cpp
  controlmethodhelper.cpp
  interface.hpp
  eventlog_helper.hpp
  controlmethodhelper.hpp
  controlserver.hpp
  ${GENERATED_CPP}
  ${GENERATED_H}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
