cmake_minimum_required( VERSION 3.11 )

project( qtplatz-windows-dependency )

get_filename_component( SOURCE_ROOT "${CMAKE_SOURCE_DIR}/../../.." ABSOLUTE )
set ( BUILD_ROOT ${CMAKE_CURRENT_BINARY_DIR} )
set ( EIGEN_SOURCE_DIR ${SOURCE_ROOT}/eigen )
set ( EIGEN_BINARY_DIR ${BUILD_ROOT}/eigen )

set ( RDBASE ${SOURCE_ROOT}/rdkit )  # <- RDKit source & install directory
set ( RDKIT_RELEASE "Release_2018_03_1" )
set ( RDKIT_BINARY_DIR "${BUILD_ROOT}/rdkit" )
set ( RDKIT_SOURCE_DIR "${RDBASE}" )
set ( NUMBER_OF_PROCESSORS $ENV{NUMBER_OF_PROCESSORS} )

file( STRINGS ${CMAKE_SOURCE_DIR}/../constants.bat configs )

foreach( i ${configs} )
  string( REGEX MATCHALL "^set[ \t]+[^=]+=.+$" config ${i} )
  if ( config )
    string( REGEX REPLACE "^set[ \t]+([^=]+)=(.+$)" "\\1" key ${i} )
    string( REGEX REPLACE "^set[ \t]+([^=]+)=(.+$)" "\\2" value ${i} )
    if ( ${key} MATCHES "^(BOOST_VERSION|QMAKE)$" )
      set( ${key} ${value} )
    endif()
  endif()
endforeach()

if ( NOT BOOST_VERSION )
  message( FATAL_ERROR "Empty BOOST_VERSION" )
endif()

set ( BOOST_SOURCE_DIR ${BUILD_ROOT}/boost_${BOOST_VERSION} )
set ( BZIP2_SOURCE_DIR ${BUILD_ROOT}/bzip2-1.0.6 )

if ( $ENV{VisualStudioVersion} VERSION_LESS "15.0" )
  message( FATAL_ERROR "Visual Studio Version $ENV{VisualStudioVersion} too old" )
else()
  set ( GENERATOR "Visual Studio 15 2017 Win64" )
endif()

if ( QMAKE )
  execute_process( COMMAND ${QMAKE} -query QT_INSTALL_PREFIX OUTPUT_VARIABLE QTDIR )
  message( STATUS "QTDIR               = " ${QTDIR} )
endif()

###########################################################
### qwt 
include ( qwt.cmake )

### boost
file ( TO_NATIVE_PATH ${BZIP2_SOURCE_DIR} BZIP2_SOURCE_PATH )

add_custom_target( boost
  COMMAND ${CMAKE_COMMAND}
  -DBOOST_VERSION=${BOOST_VERSION}
  -DBOOST_SOURCE_DIR=${BOOST_SOURCE_DIR}
  -DBZIP2_SOURCE_DIR=${BZIP2_SOURCE_DIR}
  -DDOWNLOADS=$ENV{HOME}/Downloads
  -P "${CMAKE_CURRENT_SOURCE_DIR}/boost.cmake"
  COMMENT "Run boost.bat"
  )
# -- configure_file needs to be after the add_custom_target that prevent skip tar xvf ...
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/boost.bat.in
  ${CMAKE_CURRENT_BINARY_DIR}/boost.bat
  )
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/boost-build.bat.in
  ${BOOST_SOURCE_DIR}/build.bat
  )

### eigen
add_custom_target( eigen
  COMMAND ${CMAKE_COMMAND}
  -DEIGEN_SOURCE_DIR=${EIGEN_SOURCE_DIR}
  -DEIGEN_BINARY_DIR=${EIGEN_BINARY_DIR}
  -DGENERATOR=${GENERATOR}
  -P "${CMAKE_CURRENT_SOURCE_DIR}/eigen.cmake"
  COMMENT "Run eigen.bat"
  )

### rdkit
add_custom_target( rdkit 
  COMMAND ${CMAKE_COMMAND}
  -DRDBASE=${RDBASE}
  -DRDKIT_SOURCE_DIR=${RDKIT_SOURCE_DIR}
  -DRDKIT_BINARY_DIR=${RDKIT_BINARY_DIR}
  -DRDKIT_REVISION=${RDKIT_REVISION}
  -DGENERATOR=${GENERATOR}
  -P "${CMAKE_CURRENT_SOURCE_DIR}/rdkit.cmake"
  COMMENT "Run rdkit.bat"
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/rdkit.bat.in
  ${CMAKE_CURRENT_BINARY_DIR}/rdkit.bat
  )

message( STATUS "SOURCE_ROOT         = " ${SOURCE_ROOT} )
message( STATUS "BUILD_ROOT          = " ${BUILD_ROOT} )
message( STATUS "EIGEN_SOURCE_DIR    = " ${EIGEN_SOURCE_DIR} )
message( STATUS "RDKIT_REVISION      = " ${RDKIT_REVISION} )
message( STATUS "RDKIT_BINARY_DIR    = " ${RDKIT_BINARY_DIR} )
message( STATUS "QTDIR               = " ${QTDIR} )
message( STATUS "GENERATOR           = " ${GENERATOR} )
message( STATUS )
