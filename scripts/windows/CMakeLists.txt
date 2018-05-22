cmake_minimum_required( VERSION 3.11 )

project( qtplatz-windows-dependency )

get_filename_component( SOURCE_ROOT "${CMAKE_SOURCE_DIR}/../../.." ABSOLUTE )
set ( EIGEN_SOURCE_DIR ${SOURCE_ROOT}/eigen )
set ( BUILD_ROOT ${SOURCE_ROOT}/build-vc$ENV{VisualStudioVersion}-x86_64 )

set ( RDBASE ${SOURCE_ROOT}/rdkit )  # <- RDKit source & install directory

file( STRINGS ${CMAKE_SOURCE_DIR}/../constants.bat configs )

foreach( i ${configs} )
  string( REGEX MATCHALL "^set[ \t]+[^=]+=.+$" config ${i} )
  if ( config )
    string( REGEX REPLACE "^set[ \t]+([^=]+)=(.+$)" "\\1" key ${i} )
    string( REGEX REPLACE "^set[ \t]+([^=]+)=(.+$)" "\\2" value ${i} )
    if ( ${key} MATCHES "^(BOOST_VERSION|QMAKE)$" )
      # message( STATUS "found variable: " ${key} "=" ${value} )
      set( ${key} ${value} )
    endif()
  endif()
endforeach()

set ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR} )

if ( BOOST_VERSION )
  message( STATUS "BOOST_VERSION      =" ${BOOST_VERSION} )
endif()
if ( QMAKE )
  message( STATUS "QMAKE              =" ${QMAKE} )
endif()
if ( VisualStudioVersion )
  message( STATUS "VisualStudioVersion=" ${VisualStudioVersion} )
endif()
message( STATUS "SOURCE_ROOT         =" ${SOURCE_ROOT} )
message( STATUS "BUILD_ROOT          =" ${BUILD_ROOT} )
message( STATUS "EIGEN_SOURCE_DIR    =" ${EIGEN_SOURCE_DIR} )

include ( qwt.cmake )
include ( eigen.cmake )
include ( boost.cmake )
include ( rdkit.cmake )

