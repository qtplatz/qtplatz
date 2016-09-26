#
cmake_minimum_required (VERSION 2.8.9)

project ( diag )
set( BOOST_VERSION boost-1_59 )
set( Boost_INCLUDE_DIR "/usr/local/${BOOST_VERSION}/include")
set( Boost_LIBRARY_DIR "/usr/local/${BOOST_VERSION}/lib")

if ( NOT Boost_FOUND) 
  find_package(Boost 1.57 REQUIRED COMPONENTS thread system date_time filesystem )
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#include_directories(  ${ACQRS_INCLUDE_DIRS} )
include_directories( ${Boost_INCLUDE_DIRS} )

add_definitions( -D_LINUX -D_ACQIRIS )

set( SOURCES diag.cpp )

add_executable( diag diag.cpp )

target_link_libraries( diag dl AqDrv4 )

#install( TARGETS diag RUNTIME DESTINATION bin COMPONENT applications )

