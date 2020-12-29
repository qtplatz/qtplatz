cmake_minimum_required( VERSION 3.18.2 )

project( qtplatz-windows-dependency )

if ( NOT WIN32 )
  message( FATAL_ERROR "This cmake script is targetted for win32")
endif()

list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake/Modules" )
#include ( ${CMAKE_CURRENT_LIST_DIR}/../../qtplatz.cmake )

get_filename_component( SRCDIR "${CMAKE_SOURCE_DIR}/../../.." ABSOLUTE )
set ( BUILD_ROOT ${CMAKE_CURRENT_BINARY_DIR} )
set ( NUMBER_OF_PROCESSORS $ENV{NUMBER_OF_PROCESSORS} )

#----------------- eigen ---------------
set ( EIGEN_SOURCE_DIR ${SRCDIR}/eigen )
set ( EIGEN_BINARY_DIR ${BUILD_ROOT}/eigen.build )
set ( EIGEN_INSTALL_PREFIX "c:/opt/Eigen3" )
set ( EIGEN3_INCLUDE_DIR "${EIGEN_INSTALL_PREFIX}/include/eigen3" ) # reference in rdkit-bootstrap.bat

#----------------- maeparser ---------------
set ( MAEPARSER_SOURCE_DIR ${SRCDIR}/maeparser )
set ( MAEPARSER_BINARY_DIR ${BUILD_ROOT}/maeparser.build )
set ( MAEPARSER_INSTALL_PREFIX "C:/opt/maeparser" )
set ( MAEPARSER_DIR "${MAEPARSER_INSTALL_PREFIX}" )

#----------------- rdkit --------------=
set ( RDBASE ${SRCDIR}/rdkit )  # <- RDKit source & install directory
set ( RDKIT_BINARY_DIR "${BUILD_ROOT}/rdkit.build" )
set ( RDKIT_SOURCE_DIR "${RDBASE}" )
set ( RDKIT_INSTALL_PREFIX "C:/opt/RDKit" )
if ( NOT BOOST_VERSION )
  message( FATAL_ERROR "Empty BOOST_VERSION" )
endif()

set ( BOOST_SOURCE_DIR ${BUILD_ROOT}/boost_${BOOST_VERSION} )
set ( BZIP2_SOURCE_DIR ${SRCDIR}/bzip2-1.0.6 )

set ( ZLIB_SOURCE_DIR ${SRCDIR}/zlib-1.2.11 )
set ( ZLIB_BINARY_DIR ${BUILD_ROOT}/zlib.build )
set ( ZLIB_INSTALL_PREFIX "C:/opt/zlib" )

set ( QWT_SOURCE_DIR ${SRCDIR}/qwt-6.1.5 )

if ( NOT $ENV{HOME} STREQUAL "")
  set ( DOWNLOADS $ENV{HOME}/Downloads )
  set ( HOME $ENV{HOME} )
elseif ( NOT $ENV{USERPROFILE} STREQUAL "")
  set ( DOWNLOADS $ENV{USERPROFILE}/Downloads )
  set ( HOME $ENV{USERPROFILE} )
else()
  message( FATAL_ERROR "No user home directory found" )
endif()

if ( $ENV{VisualStudioVersion} VERSION_LESS "15.0" )
  message( FATAL_ERROR "Visual Studio Version $ENV{VisualStudioVersion} too old" )
elseif ( $ENV{VisualStudioVersion} VERSION_EQUAL "15.0" )
  set ( GENERATOR "Visual Studio 15 2017 Win64" )
elseif ( $ENV{VisualStudioVersion} VERSION_EQUAL "16.0" )
  set ( GENERATOR "Visual Studio 16 2019" )
else()
  message( FATAL_ERROR "Visual Studio Version $ENV{VisualStudioVersion} not supported" )
endif()

if ( QMAKE )
  execute_process( COMMAND ${QMAKE} -query QT_INSTALL_PREFIX OUTPUT_VARIABLE QTDIR )
endif()

# force set BOOST_ROOT, otherwise pick up most recent version of boost installed
if ( BOOST_VERSION )
  string ( REGEX REPLACE "([0-9]+)_([0-9]+)_[0-9]+.*$" "boost-\\1_\\2" __boost_root ${BOOST_VERSION} )
  set ( BOOST_ROOT "c:/Boost/include/${__boost_root}" )
endif()

if ( OFF )
  message( STATUS "##########################################" )
  if ( EXISTS ${BOOST_ROOT} )
    #  set ( Boost_DEBUG 1 )
    set ( BOOST_NO_SYSTEM_PATHS ON )
    message( STATUS "BOOST_ROOT: ${BOOST_ROOT} -- exists" )
    message( STATUS "BOOST_LIBRARYDIR: ${BOOST_LIBRARYDIR}" )
    message( STATUS "BOOST_NO_SYSTEM_PATHS: ${BOOST_NO_SYSTEM_PATHS}" )
    find_package( Boost ) #1.69 REQUIRED COMPONENTS program_options )
    if ( Boost_FOUND )
      find_package( Boost 1.73 REQUIRED COMPONENTS program_options )
      message( STATUS "BOOST_VERSION: ${Boost_VERSION}")
      message( STATUS "BOOST_INCLUDE_DIR: ${Boost_INCLUDE_DIR}")
      message( STATUS "BOOST_LIBS: ${Boost_LIBRARIES}")
    else()
      message( STATUS "##########################################" )
      message( FATAL_ERROR "BOOST_VERSION: ${Boost_VERSION}")
    endif()
    return()
  endif()
endif()

###########################################################
####################### python #############################

set ( RDK_BUILD_PYTHON_WRAPPERS "OFF" )
find_package( Python3 )

if ( Python3_FOUND )
    execute_process( COMMAND
      ${Python3_EXECUTABLE} -c "import numpy ; print( numpy.get_include() );"
      OUTPUT_VARIABLE PYTHON_NUMPY_INCLUDE_PATH
      OUTPUT_STRIP_TRAILING_WHITESPACE )

    execute_process( COMMAND
      ${Python3_EXECUTABLE} -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])"
      OUTPUT_VARIABLE PYTHON_INCLUDE
      OUTPUT_STRIP_TRAILING_WHITESPACE )

    execute_process( COMMAND
      ${Python3_EXECUTABLE} -c "from sysconfig import get_paths as gp; print(gp()[\"data\"])"
      OUTPUT_VARIABLE PYTHON_ROOT
      OUTPUT_STRIP_TRAILING_WHITESPACE  )

    execute_process( COMMAND
      ${Python3_EXECUTABLE} -c "import sys; print(sys.executable)"
      OUTPUT_VARIABLE PYTHON
      OUTPUT_STRIP_TRAILING_WHITESPACE  )

    execute_process( COMMAND
      ${Python3_EXECUTABLE} -c "import sys; print('{}.{}'.format(*sys.version_info))"
      OUTPUT_VARIABLE PYTHON_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE )

    if ( PYTHON_NUMPY_INCLUDE_PATH )
      # uncomment following line to enable PYTHON_WRAPPERS build
      set ( RDK_BUILD_PYTHON_WRAPPERS "ON" )
      set ( PYTHON_NUMPY_INCLUDE_PATH ${PYTHON_NUMPY_INCLUDE_PATH} )
    endif()

endif()

###########################################################
### qwt

include ( qwt.cmake )

#########################################################
######################## boost ##########################

add_custom_target( boost
  COMMAND ${CMAKE_COMMAND}
  -DCURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
  -DBOOST_VERSION=${BOOST_VERSION}
  -DBOOST_SOURCE_DIR=${BOOST_SOURCE_DIR}
  -DBZIP2_SOURCE_DIR=${BZIP2_SOURCE_DIR}
  -DZLIB_SOURCE_DIR=${ZLIB_SOURCE_DIR}
  -DDOWNLOADS=${DOWNLOADS}
  -P "${CMAKE_CURRENT_SOURCE_DIR}/boost.cmake"
  COMMAND cmd /c "echo ####################################################################################"
  COMMAND cmd /c "echo boost.cmake done."
  COMMAND cmd /c "echo ####################################################################################"
  COMMAND cmd /c "cd ${BOOST_SOURCE_DIR} && bootstrap.bat"
  COMMAND cmd /c "echo ####################################################################################"
  COMMAND cmd /c "cd ${BOOST_SOURCE_DIR} && boost-build.bat"
  COMMAND cmd /c "echo ####################################################################################"
  COMMAND cmd /c "echo boost bootstrap.bat completed"
  COMMAND cmd /c "echo ####################################################################################"
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/user-config.jam.in
  ${HOME}/user-config.jam
  )

###################### eigen #########################
add_custom_target( eigen.download
  COMMAND ${CMAKE_COMMAND}
  -DGIT_CLONE_DIR="${EIGEN_SOURCE_DIR}"
  -DGIT_CLONE_URL="https://github.com/eigenteam/eigen-git-mirror"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/git.cmake"
  )

add_custom_target( eigen
  COMMAND cmd /c "eigen-bootstrap.bat"
  COMMAND cmd /c "cd ${EIGEN_BINARY_DIR} && nmake install"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR}
  DEPENDS eigen.download
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/eigen-bootstrap.bat.in
  ${CMAKE_CURRENT_BINARY_DIR}/eigen-bootstrap.bat
  )

###################### maeparser #########################
add_custom_target( maeparser.download
  COMMAND ${CMAKE_COMMAND}
  -DGIT_CLONE_DIR="${MAEPARSER_SOURCE_DIR}"
  -DGIT_CLONE_URL="https://github.com/schrodinger/maeparser"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/git.cmake"
  )

add_custom_target( maeparser
  COMMAND cmd /c "maeparser-bootstrap.bat"
  COMMAND cmd /c "cd ${MAEPARSER_BINARY_DIR} && nmake install"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR}
  DEPENDS maeparser.download
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/maeparser-bootstrap.bat.in
  ${CMAKE_CURRENT_BINARY_DIR}/maeparser-bootstrap.bat
  )

######################### zlib #######################
add_custom_target( zlib
  COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX="${ZLIB_INSTALL_PREFIX}" -G "${GENERATOR}" "${ZLIB_SOURCE_DIR}"
  WORKING_DIRECTORY ${ZLIB_BINARY_DIR}
  )

if ( NOT EXISTS ${ZLIB_BINARY_DIR} )
  file( MAKE_DIRECTORY ${ZLIB_BINARY_DIR} )
endif()

###########################################################
####################### rdkit #############################
set( RDK_BUILD_MAEPARSER_SUPPORT "OFF" )
find_path( maeparser_dirs
  NAMES "maeparser/Reader.hpp" HINTS ${MAEPARSER_INSTALL_PREFIX} PATH_SUFFIXES "include" )
if ( maeparser_dirs )
  set( RDK_BUILD_MAEPARSER_SUPPORT "ON" )
endif()

add_custom_target( rdkit.download
  COMMAND ${CMAKE_COMMAND}
  -DGIT_CLONE_DIR="${RDBASE}"
  -DGIT_CLONE_URL="https://github.com/rdkit/rdkit"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/git.cmake"
  )

add_custom_target( rdkit
  COMMAND cmd /c "rdkit-bootstrap.bat"
  COMMAND cmd /c "cd ${RDKIT_BINARY_DIR} && nmake install"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR}
  DEPENDS rdkit.download
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/rdkit-bootstrap.bat.in
  ${CMAKE_CURRENT_BINARY_DIR}/rdkit-bootstrap.bat
  )

message( STATUS "SRCDIR              = " ${SRCDIR} )
message( STATUS "BUILD_ROOT          = " ${BUILD_ROOT} )
message( STATUS "EIGEN_SOURCE_DIR    = " ${EIGEN_SOURCE_DIR} )
message( STATUS "RDKIT_REVISION      = " ${RDKIT_REVISION} )
message( STATUS "RDKIT_BINARY_DIR    = " ${RDKIT_BINARY_DIR} )
message( STATUS "BOOST_VERSION       = " ${BOOST_VERSION} )
message( STATUS "BOOST_ROOT          = " ${BOOST_ROOT} )
message( STATUS "BOOST_SOURCE_DIR    = " ${BOOST_SOURCE_DIR} )
message( STATUS "QTDIR               = " ${QTDIR} )
message( STATUS "GENERATOR           = " ${GENERATOR} )
message( STATUS "Python3_FOUND       = " ${Python3_FOUND} )
if ( Python3_FOUND )
  message( STATUS "\tEXECUTABLE     = " ${Python3_EXECUTABLE} )
  message( STATUS "\tNUMPY_INCLUDE  = " ${PYTHON_NUMPY_INCLUDE_PATH} )
endif()
