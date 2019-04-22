cmake_minimum_required( VERSION 3.11 )

project( qtplatz-windows-dependency )

if ( NOT WIN32 )
  message( FATAL_ERROR "This cmake script is targetted for win32")
endif()

include (../../qtplatz.cmake)

get_filename_component( SOURCE_ROOT "${CMAKE_SOURCE_DIR}/../../.." ABSOLUTE )
set ( BUILD_ROOT ${CMAKE_CURRENT_BINARY_DIR} )
set ( EIGEN_SOURCE_DIR ${BUILD_ROOT}/eigen )
set ( EIGEN_BINARY_DIR ${BUILD_ROOT}/eigen.build )

set ( RDBASE ${SOURCE_ROOT}/rdkit )  # <- RDKit source & install directory
set ( RDKIT_RELEASE "Release_2019_03_1" )
set ( RDKIT_BINARY_DIR "${BUILD_ROOT}/rdkit" )
set ( RDKIT_SOURCE_DIR "${RDBASE}" )
set ( NUMBER_OF_PROCESSORS $ENV{NUMBER_OF_PROCESSORS} )

include( ${CMAKE_SOURCE_DIR}/../constants.cmake )

if ( NOT BOOST_VERSION )
  message( FATAL_ERROR "Empty BOOST_VERSION" )
endif()

set ( BOOST_SOURCE_DIR ${BUILD_ROOT}/boost_${BOOST_VERSION} )
set ( BZIP2_SOURCE_DIR ${BUILD_ROOT}/bzip2-1.0.6 )
set ( ZLIB_SOURCE_DIR ${BUILD_ROOT}/zlib-1.2.11 )
set ( QWT_SOURCE_DIR ${BUILD_ROOT}/qwt-6.1.3 )
set ( DOWNLOADS $ENV{HOME}/Downloads )

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
file ( TO_NATIVE_PATH ${BZIP2_SOURCE_DIR} BZIP2_SOURCE_PATH ) # use in boost-build.bat.in
file ( TO_NATIVE_PATH ${ZLIB_SOURCE_DIR} ZLIB_SOURCE_PATH ) # use in boost-build.bat.in

add_custom_target( boost
  COMMAND ${CMAKE_COMMAND}
  -DBOOST_VERSION=${BOOST_VERSION}
  -DBOOST_SOURCE_DIR=${BOOST_SOURCE_DIR}
  -DBZIP2_SOURCE_DIR=${BZIP2_SOURCE_DIR}
  -DZLIB_SOURCE_DIR=${ZLIB_SOURCE_DIR}  
  -DDOWNLOADS=${DOWNLOADS}
  -P "${CMAKE_CURRENT_SOURCE_DIR}/boost.cmake"
  COMMAND cmd /c "boost.bat"
  )

# -- configure_file needs to be after the add_custom_target that prevent skip tar xvf ...
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/boost.bat.in
  ${CMAKE_CURRENT_BINARY_DIR}/boost.bat
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/boost-build.bat.in
  ${BOOST_SOURCE_DIR}/boost-build.bat
  )

### eigen
add_custom_target( eigen
  COMMAND ${CMAKE_COMMAND}
  -DEIGEN_SOURCE_DIR=${EIGEN_SOURCE_DIR}
  -DEIGEN_BINARY_DIR=${EIGEN_BINARY_DIR}
  -DGENERATOR="NMake Makefiles"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/eigen.cmake"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR}
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
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR}  
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/rdkit.bat.in
  ${CMAKE_CURRENT_BINARY_DIR}/rdkit.bat
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/eigen.bat.in
  ${CMAKE_CURRENT_BINARY_DIR}/eigen.bat
  )

message( STATUS "SOURCE_ROOT         = " ${SOURCE_ROOT} )
message( STATUS "BUILD_ROOT          = " ${BUILD_ROOT} )
message( STATUS "EIGEN_SOURCE_DIR    = " ${EIGEN_SOURCE_DIR} )
message( STATUS "RDKIT_REVISION      = " ${RDKIT_REVISION} )
message( STATUS "RDKIT_BINARY_DIR    = " ${RDKIT_BINARY_DIR} )
message( STATUS "BOOST_VERSION       = " ${BOOST_VERSION} )
message( STATUS "QTDIR               = " ${QTDIR} )
message( STATUS "GENERATOR           = " ${GENERATOR} )
message( STATUS )
