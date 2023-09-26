cmake_minimum_required( VERSION 3.22.1 ) # for VS2022

project( qtplatz-windows-dependency )

if ( NOT WIN32 )
  message( FATAL_ERROR "This cmake script is targetted for win32")
endif()

list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake/Modules" )
#include ( ${CMAKE_CURRENT_LIST_DIR}/../../qtplatz.cmake )

get_filename_component( SRCDIR "${CMAKE_SOURCE_DIR}/../../.." ABSOLUTE )
set ( BUILD_ROOT ${CMAKE_CURRENT_BINARY_DIR} )
set ( NUMBER_OF_PROCESSORS $ENV{NUMBER_OF_PROCESSORS} )

find_program (TAR "tar" HINTS "C:/msys64/usr/bin/")

#----------------- eigen ---------------
set ( EIGEN_SOURCE_DIR ${SRCDIR}/eigen )
set ( EIGEN_BINARY_DIR ${BUILD_ROOT}/eigen.build )
set ( EIGEN_INSTALL_PREFIX "C:/opt/Eigen3" ) # -- default was $ENV{ProgramFiles(x86)}
set ( EIGEN3_INCLUDE_DIR "${EIGEN_INSTALL_PREFIX}/include/eigen3" ) # reference in rdkit-bootstrap.bat

#----------------- maeparser ---------------
# RDKit 2020_09_3 (Q3 2020) Release require maeparser shared object as reported in https://github.com/rdkit/rdkit/issues/3648
# ----------- zlib ------------
#    ---------------- on build-x86_64/windows directory
#       cmake -DCMAKE_INSTALL_PREFIX=c:/opt ..\..\..\zlib-1.2.13
#       cmake --build . --config Release
#       cmake --install .
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

#----------------- opencv --------------
set ( OPENCV_SOURCE_DIR "${SRCDIR}/opencv" )
set ( OPENCV_CONTRIB_SOURCE_DIR "${SRCDIR}/opencv_contrib" )
set ( OPENCV_EXTRA_SOURCE_DIR "${SRCDIR}/opencv_extra" )

set ( OPENCV_BINARY_DIR "${BUILD_ROOT}/opencv.build" )
set ( OPENCV_INSTALL_PREFIX "C:/opt/OpenCV" )

#--------------------------------------------

set ( BOOST_SOURCE_DIR "${BUILD_ROOT}/boost_${BOOST_VERSION}" )
set ( BZIP2_SOURCE_DIR "${SRCDIR}/bzip2-1.0.6" )

set ( ZLIB_SOURCE_DIR "${SRCDIR}/zlib-1.3" )
set ( ZLIB_BINARY_DIR "${BUILD_ROOT}/zlib.build" )
set ( ZLIB_INSTALL_PREFIX "C:/opt/zlib" )

set ( QWT_VERSION "6.2.0" )
set ( QWT_SOURCE_DIR ${SRCDIR}/qwt-${QWT_VERSION} )

if ( NOT $ENV{HOME} STREQUAL "")
  set ( DOWNLOADS $ENV{HOME}/Downloads )
  set ( HOME $ENV{HOME} )
elseif ( NOT $ENV{USERPROFILE} STREQUAL "")
  set ( DOWNLOADS $ENV{USERPROFILE}/Downloads )
  set ( HOME $ENV{USERPROFILE} )
else()
  message( FATAL_ERROR "No user home directory found" )
endif()

if ( $ENV{VisualStudioVersion} VERSION_EQUAL "16.0" )
  set ( GENERATOR "Visual Studio 16 2019" )
  set ( BJAM_TOOLSET "msvc-14.2" )
elseif ( $ENV{VisualStudioVersion} VERSION_EQUAL "17.0" )
  set ( GENERATOR "Visual Studio 17 2022" )
  set ( BJAM_TOOLSET "msvc-14.3" )
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
  set ( BOOST_LIBRARYDIR "C:/Boost/lib" )
endif()

###########################################################
####################### python #############################

set ( RDK_BUILD_PYTHON_WRAPPERS "OFF" )
find_package( Python3 )

if ( Python3_FOUND )
    file ( TO_CMAKE_PATH ${Python3_EXECUTABLE} PYTHON3_EXECUTABLE )

    execute_process( COMMAND
      ${Python3_EXECUTABLE} -c "import numpy ; print( numpy.get_include() );"
      OUTPUT_VARIABLE PYTHON_NUMPY_INCLUDE_NATIVE_PATH
      OUTPUT_STRIP_TRAILING_WHITESPACE )

    file ( TO_CMAKE_PATH ${PYTHON_NUMPY_INCLUDE_NATIVE_PATH} PYTHON_NUMPY_INCLUDE_DIR )

    execute_process( COMMAND
      ${Python3_EXECUTABLE} -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])"
      OUTPUT_VARIABLE PYTHON_INCLUDE_NATIVE_PATH
      OUTPUT_STRIP_TRAILING_WHITESPACE )
    file ( TO_CMAKE_PATH ${PYTHON_INCLUDE_NATIVE_PATH} PYTHON_INCLUDE_DIR )

    execute_process( COMMAND
      ${Python3_EXECUTABLE} -c "from sysconfig import get_paths as gp; print(gp()[\"data\"])"
      OUTPUT_VARIABLE PYTHON_ROOT
      OUTPUT_STRIP_TRAILING_WHITESPACE  )


    execute_process( COMMAND
      ${Python3_EXECUTABLE} -c "import sys; print('{}.{}'.format(*sys.version_info))"
      OUTPUT_VARIABLE PYTHON_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE )

    if ( PYTHON_NUMPY_INCLUDE_DIR )
      set ( RDK_BUILD_PYTHON_WRAPPERS "ON" )
    endif()

    set ( OCV_BUILD_PYTHON3 "OFF" )

endif()

###########################################################
######################### qwt #############################

add_custom_target( qwt.download
  COMMAND ${CMAKE_COMMAND}
  -DDOWNLOAD_URL="https://sourceforge.net/projects/qwt/files/qwt/${QWT_VERSION}/qwt-${QWT_VERSION}.tar.bz2/download"
  -DDOWNLOAD_FILE="qwt-${QWT_VERSION}.tar.bz2"
  -DDOWNLOAD_SOURCE_DIR="${QWT_SOURCE_DIR}"
  -DDOWNLOAD_FORMAT="tar"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/download.cmake"
  )

add_custom_target( qwtconfig
  COMMAND ${CMAKE_COMMAND}
  -P "${CMAKE_CURRENT_SOURCE_DIR}/qwtconfig.cmake"
  WORKING_DIRECTORY ${QWT_SOURCE_DIR}
  DEPENDS qwt.download
)

add_custom_target( qwt
  COMMAND cmd /c qwt-build.bat
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS qwt.download qwtconfig
  )

file ( TO_NATIVE_PATH ${QWT_SOURCE_DIR} QWT_SOURCE_NATIVE_DIR )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/qwt-build.bat.in
  ${CMAKE_CURRENT_BINARY_DIR}/qwt-build.bat
  )

###########################################################

#########################################################
######################## boost ##########################

add_custom_target( boost
  COMMAND ${CMAKE_COMMAND}
  -DCURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
  -DBOOST_VERSION="${BOOST_VERSION}"
  -DBOOST_SOURCE_DIR="${BOOST_SOURCE_DIR}"
  -DBZIP2_SOURCE_DIR="${BZIP2_SOURCE_DIR}"
  -DZLIB_SOURCE_DIR="${ZLIB_SOURCE_DIR}"
  -DZLIB_BINARY_DIR="${ZLIB_BINARY_DIR}"
  -DZLIB_INSTALL_PREFIX="${ZLIB_INSTALL_PREFIX}"
  -DDOWNLOADS="${DOWNLOADS}"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/boost.cmake"
  COMMAND cmd /c "cd ${BOOST_SOURCE_DIR} && bootstrap.bat"
  COMMAND cmd /c "echo ####################################################################################"
  COMMAND cmd /c "echo boost.cmake done."
  COMMAND cmd /c "echo ####################################################################################"
  COMMAND cmd /c "echo ======================= check zconf.h is configured ================================"
  COMMAND cmd /c "echo ==== zlib source need to be pre-configured for zconf.h"
  COMMAND cmd /c "echo ==== go to zlib source dir: ${ZLIB_SOURCE_DIR}"
  COMMAND cmd /c "echo ==== mkdir build; cmake .."
  COMMAND cmd /c "echo ==== copy zconf."
  COMMAND cmd /c "echo ####################################################################################"
  COMMAND cmd /c "echo boost-build.bat static dynamic"
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
  COMMAND ${CMAKE_COMMAND} --build "${EIGEN_BINARY_DIR}" --config Release --target install
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
  COMMAND ${CMAKE_COMMAND} --build "${MAEPARSER_BINARY_DIR}" --config Release --target install
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR}
  DEPENDS maeparser.download
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/maeparser-bootstrap.bat.in
  ${CMAKE_CURRENT_BINARY_DIR}/maeparser-bootstrap.bat
  )

######################### zlib #######################
add_custom_target( zlib
  COMMAND ${CMAKE_COMMAND}
  -DCMAKE_INSTALL_PREFIX="${ZLIB_INSTALL_PREFIX}"
  -G "${GENERATOR}" "${ZLIB_SOURCE_DIR}"
  WORKING_DIRECTORY "${ZLIB_BINARY_DIR}"
)

if ( NOT EXISTS ${ZLIB_BINARY_DIR} )
  file( MAKE_DIRECTORY ${ZLIB_BINARY_DIR} )
endif()

###########################################################
####################### rdkit #############################
# if MAEPARSER_SUPPORT to ON, whole system will has a dependency to the maeparser.dll
set( RDK_BUILD_MAEPARSER_SUPPORT "OFF" )
find_path( maeparser_dirs
  NAMES "maeparser/Reader.hpp" HINTS ${MAEPARSER_INSTALL_PREFIX} PATH_SUFFIXES "include" )
if ( maeparser_dirs )
  set( RDK_BUILD_MAEPARSER_SUPPORT "ON" )
  list( APPEND CMAKE_MODULE_PATH "${MAEPARSER_INSTALL_PREFIX}/lib" )
  find_package( maeparser )
endif()


add_custom_target( rdkit.download  COMMAND ${CMAKE_COMMAND}
  -DGIT_CLONE_DIR="${RDBASE}"
  -DGIT_CLONE_URL="https://github.com/rdkit/rdkit"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/git.cmake"
  )

add_custom_target( rdkit
  COMMAND cmd /c "rdkit-bootstrap.bat"
  COMMAND ${CMAKE_COMMAND} --build "${RDKIT_BINARY_DIR}" --config Release --target install -- /m
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR}
  DEPENDS rdkit.download
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/rdkit-bootstrap.bat.in
  ${CMAKE_CURRENT_BINARY_DIR}/rdkit-bootstrap.bat
  )

###########################################################
####################### opencv #############################

add_custom_target( opencv.download
  COMMAND ${CMAKE_COMMAND}
  -DGIT_CLONE_DIR="${OPENCV_SOURCE_DIR}"
  -DGIT_CLONE_URL="https://github.com/opencv/opencv.git"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/git.cmake"
  )

add_custom_target( opencv_contrib.download
  COMMAND ${CMAKE_COMMAND}
  -DGIT_CLONE_DIR="${OPENCV_CONTRIB_SOURCE_DIR}"
  -DGIT_CLONE_URL="https://github.com/opencv/opencv_contrib.git"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/git.cmake"
  )

add_custom_target( opencv_extra.download
  COMMAND ${CMAKE_COMMAND}
  -DGIT_CLONE_DIR="${OPENCV_EXTRA_SOURCE_DIR}"
  -DGIT_CLONE_URL="https://github.com/opencv/opencv_extra.git"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/git.cmake"
  )

add_custom_target( opencv
  COMMAND cmd /c "opencv-bootstrap.bat"
  COMMAND cmd /c "cd ${OPENCV_BINARY_DIR} && build-release.bat"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR}
  DEPENDS opencv.download opencv_contrib.download opencv_extra.download
  )

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/opencv-bootstrap.bat.in
  ${CMAKE_CURRENT_BINARY_DIR}/opencv-bootstrap.bat
  )

###########################################################
message( STATUS "TAR                 = " ${TAR} )
message( STATUS "SRCDIR              = " ${SRCDIR} )
message( STATUS "BUILD_ROOT          = " ${BUILD_ROOT} )
message( STATUS "EIGEN_SOURCE_DIR    = " ${EIGEN_SOURCE_DIR} )
message( STATUS "RDKIT_REVISION      = " ${RDKIT_REVISION} )
message( STATUS "RDKIT_BINARY_DIR    = " ${RDKIT_BINARY_DIR} )
message( STATUS "BOOST_VERSION       = " ${BOOST_VERSION} )
message( STATUS "BOOST_ROOT          = " ${BOOST_ROOT} )
message( STATUS "BOOST_SOURCE_DIR    = " ${BOOST_SOURCE_DIR} )
message( STATUS "MAEPARSER_SUPPORT   = " ${RDK_BUILD_MAEPARSER_SUPPORT} )
message( STATUS "MAEPARSER_DIRS      = " ${maeparser_dirs} )
message( STATUS "QTDIR               = " ${QTDIR} )
message( STATUS "GENERATOR           = " ${GENERATOR} )
message( STATUS "Python3_FOUND       = " ${Python3_FOUND} )
if ( Python3_FOUND )
  message( STATUS "\tEXECUTABLE     = " ${PYTHON3_EXECUTABLE} "(" ${Python3_EXECUTABLE} ")")
  message( STATUS "\tNUMPY_INCLUDE  = " ${PYTHON_NUMPY_INCLUDE_DIR} "(" ${PYTHON_NUMPY_INCLUDE_PATH} ")")
endif()
