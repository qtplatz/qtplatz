#
cmake_minimum_required (VERSION 2.8.9)

project ( test_u5303a )

set( QTPLATZ_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../../.. )

include( "../../../qtplatz.cmake" )

find_package(Boost 1.57 REQUIRED COMPONENTS thread system date_time filesystem )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(
  ${QTPLATZ_SOURCE_DIR}/src/libs
  ${QTPLATZ_SOURCE_DIR}/contrib/agilent/libs
  ${Boost_INCLUDE_DIRS}
  )

set( SOURCES test_u5303a.cpp )

add_executable( test_u5303a main.cpp )

link_directories( ${QTPLATZ_SOURCE_DIR}/../build-x86_64/qtplatz.debug/lib/qtplatz )

target_link_libraries( ${PROJECT_NAME}
  /home/toshi/src/build-x86_64/qtplatz.debug/lib/qtplatz/libu5303a.so
  /home/toshi/src/build-x86_64/qtplatz.debug/lib/qtplatz/libacqrscontrols.so
  AgMD2 dl )


