#
cmake_minimum_required (VERSION 2.8.9)

project ( test_u5303a )

find_package( Boost REQUIRED COMPONENTS system )

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

find_package(Boost 1.57 REQUIRED COMPONENTS program_options thread system date_time filesystem )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set( SOURCES test_u5303a.cpp )

add_executable( test_u5303a main.cpp )

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
  ${Boost_LIBRARIES}
  AgMD2
  ${DL} )


