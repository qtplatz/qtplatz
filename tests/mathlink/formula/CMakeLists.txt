cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( formula )

set ( CMAKE_CXX_STANDARD 14 )

set( QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../.. )
include( ${QTPLATZ_SOURCE_DIR}/../build-Linux-x86_64/qtplatz.debug/qtplatz-config.cmake )

find_package( Boost REQUIRED COMPONENTS system )
find_package( rdkit )

include_directories( ${WSTP_INCLUDE_DIRS} )
link_directories( ${WSTP_LIBRARY_DIRS} )

include_directories(
  ${QMAKE_SOURCE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

if ( rdkit_FOUND )
  add_definitions( -DHAVE_RDKit=1 )
  include_directories( ${RDKit_INCLUDE_DIRS} )
  message( STATUS "RDkit defined" )
endif()

add_definitions(-DADCONTROLS_LIBRARY) # fake for direct linking adcontrols.dll soruce code

WSTP_ADD_TM(formula.tm .c)

add_executable ( ${PROJECT_NAME}
  formula.tm.c
  main.cpp
  formula.cpp
  )

target_link_libraries(
  ${PROJECT_NAME}
  LINK_PUBLIC
  adportable
  adcontrols
  ${WSTP_LIBRARIES}
  ${WSTP_SYSTEM_LIBRARIES}
  ${RDKit_LIBRARIES}
  ${Boost_LIBRARIES}
  )
