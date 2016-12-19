
set ( QTPLATZ_BUILD_DIR ${QTPLATZ_BUILD_DIR} CACHE STRING "destination directory" )

if ( NOT ${CMAKE_BUILD_TYPE} STREQUAL "" )
  string( TOLOWER ${CMAKE_BUILD_TYPE} build_type )
else()
  set( build_type "release" )
endif()

find_file( qtplatz_config NAME "qtplatz-config.cmake" PATHS "${QTPLATZ_BUILD_DIR}" )

if ( qtplatz_config )
  include( ${qtplatz_config} )
endif()

include_directories( ${CMAKE_SOURCE_DIR}/../../libs )                      # qtplatz/src/libs
include_directories( ${CMAKE_SOURCE_DIR}/../../../contrib/agilent/libs )   # qtplatz/contrib/agilent/libs

set( standalone_additional_sources
  ${CMAKE_SOURCE_DIR}/../../../contrib/agilent/libs/acqrscontrols/acqiris_client.cpp
  ${CMAKE_SOURCE_DIR}/../../../contrib/agilent/libs/acqrscontrols/acqiris_client.hpp
  ${CMAKE_SOURCE_DIR}/../../../contrib/agilent/libs/acqrscontrols/acqiris_method.cpp
  ${CMAKE_SOURCE_DIR}/../../../contrib/agilent/libs/acqrscontrols/acqiris_method.hpp
  ${CMAKE_SOURCE_DIR}/../../../contrib/agilent/libs/acqrscontrols/acqiris_protocol.cpp
  ${CMAKE_SOURCE_DIR}/../../../contrib/agilent/libs/acqrscontrols/acqiris_protocol.hpp
  ${CMAKE_SOURCE_DIR}/../../../contrib/agilent/libs/acqrscontrols/acqiris_waveform.cpp
  ${CMAKE_SOURCE_DIR}/../../../contrib/agilent/libs/acqrscontrols/acqiris_waveform.hpp
  )
  
