#
cmake_minimum_required (VERSION 2.8.9)

project ( diag-u5303a )

find_package( Boost REQUIRED COMPONENTS system iostreams bzip2 )

if ( WIN32 )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS )
  set( IVIROOTDIR "C:/Program Files/IVI Foundation/IVI" )
  set( VXIPNPPATH "C:/Program Files/IVI Foundation/VISA" )
  include_directories(  ${IVIROOTDIR}/Include  )
  find_library( AgMD2_LIBRARY AgMD2 ${IVIROOTDIR}/Lib_x64/msc )
else()
  set( AgMD2_LIBRARY "AgMD2" )
endif()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${QTPLATZ_SOURCE_DIR}/contrib/agilent/libs
  ${QTPLATZ_SOURCE_DIR}/contrib/linux
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

find_package(Boost 1.57 REQUIRED COMPONENTS program_options thread system date_time filesystem )

set( SOURCES test_u5303a.cpp )

add_executable( ${PROJECT_NAME} main.cpp )

link_directories( ${QTPLATZ_SOURCE_DIR}/../build-x86_64/qtplatz.debug/lib/qtplatz )

if ( ${CMAKE_SYSTEM} MATCHES "Linux" )
  set( DL dl )
else()
  set( DL )
endif()

target_link_libraries( ${PROJECT_NAME}
  u5303a
  adcontrols
  acqrscontrols
  dgpio
  ${Boost_LIBRARIES}
  ${AgMD2_LIBRARY}
  ${DL} )


