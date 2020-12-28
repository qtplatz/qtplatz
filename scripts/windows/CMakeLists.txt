cmake_minimum_required( VERSION 3.18.2 )

project( qtplatz-windows-dependency )

message( STATUS "**************************************" )
message( STATUS "** " ${CMAKE_CURRENT_LIST_DIR}/CMakeList.txt " **")
message( STATUS "**************************************" )
if ( NOT WIN32 )
  message( FATAL_ERROR "This cmake script is targetted for win32")
endif()

list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake/Modules" )
include ( ${CMAKE_CURRENT_LIST_DIR}/../../qtplatz.cmake )

get_filename_component( SOURCE_ROOT "${CMAKE_SOURCE_DIR}/../../.." ABSOLUTE )
set ( BUILD_ROOT ${CMAKE_CURRENT_BINARY_DIR} )
set ( NUMBER_OF_PROCESSORS $ENV{NUMBER_OF_PROCESSORS} )

#----------------- eigen ---------------
set ( EIGEN_SOURCE_DIR ${SOURCE_ROOT}/eigen )
set ( EIGEN_BINARY_DIR ${BUILD_ROOT}/eigen.build )
set ( EIGEN_INSTALL_PREFIX "C:/opt/Eigen3" )

#----------------- maeparser ---------------
set ( MAEPARSER_SOURCE_DIR ${SOURCE_ROOT}/maeparser )
set ( MAEPARSER_BINARY_DIR ${BUILD_ROOT}/maeparser.build )
set ( MAEPARSER_INSTALL_PREFIX "C:/opt/maeparser" )

#----------------- rdkit --------------=
set ( RDBASE ${SOURCE_ROOT}/rdkit )  # <- RDKit source & install directory
set ( RDKIT_BINARY_DIR "${BUILD_ROOT}/rdkit.build" )
set ( RDKIT_SOURCE_DIR "${RDBASE}" )
set ( RDKIT_INSTALL_PREFIX "C:/opt/RDKit" )
if ( NOT BOOST_VERSION )
  message( FATAL_ERROR "Empty BOOST_VERSION" )
endif()

set ( BOOST_SOURCE_DIR ${BUILD_ROOT}/boost_${BOOST_VERSION} )
set ( BZIP2_SOURCE_DIR ${BUILD_ROOT}/bzip2-1.0.6 )
set ( ZLIB_SOURCE_DIR ${BUILD_ROOT}/zlib-1.2.11 )
set ( QWT_SOURCE_DIR ${BUILD_ROOT}/qwt-6.1.5 )
if ( NOT $ENV{HOME} STREQUAL "")
  set ( DOWNLOADS $ENV{HOME}/Downloads )
elseif ( NOT $ENV{USERPROFILE} STREQUAL "")
  set ( DOWNLOADS $ENV{USERPROFILE}/Downloads )
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
  string ( REGEX REPLACE "([0-9]+)_([0-9]+)_[0-9]+.*$" "boost-\\1.\\2" __boost_root ${BOOST_VERSION} )
  set ( BOOST_ROOT "c:/Boost/include/${__boost_root}" )
endif()

###########################################################
### qwt

include ( qwt.cmake )

###################### boost #########################
file ( TO_NATIVE_PATH ${BZIP2_SOURCE_DIR} BZIP2_SOURCE_PATH ) # use in boost-build.bat.in
file ( TO_NATIVE_PATH ${ZLIB_SOURCE_DIR} ZLIB_SOURCE_PATH ) # use in boost-build.bat.in

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

###################### eigen #########################
add_custom_target( eigen
  COMMAND ${CMAKE_COMMAND}
  -DCURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
  -DEIGEN_SOURCE_DIR=${EIGEN_SOURCE_DIR}
  -DEIGEN_BINARY_DIR=${EIGEN_BINARY_DIR}
  -DEIGEN_INSTALL_PREFIX="${EIGEN_INSTALL_PREFIX}"
  -DGENERATOR="NMake Makefiles"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/eigen.cmake"
  COMMAND cmd /c "cd ${EIGEN_BINARY_DIR} && bootstrap.bat"
  COMMAND cmd /c "cd ${EIGEN_BINARY_DIR} && nmake install"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR}
  )

###################### maeparser #########################

add_custom_target( maeparser
  COMMAND ${CMAKE_COMMAND}
  -DCURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
  -DMAEPARSER_SOURCE_DIR=${MAEPARSER_SOURCE_DIR}
  -DMAEPARSER_BINARY_DIR=${MAEPARSER_BINARY_DIR}
  -DCMAKE_INSTALL_PREFIX=${MAEPARSER_INSTALL_PREFIX}
  -DGENERATOR="NMake Makefiles"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/maeparser.cmake"
  COMMAND cmd /c "cd ${MAEPARSER_BINARY_DIR} && bootstrap.bat"
  COMMAND cmd /c "cd ${MAEPARSER_BINARY_DIR} && nmake install"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR}
  )

########################################33

set ( RDK_BUILD_PYTHON_WRAPPERS "OFF" )
find_package( Python3 )

if ( Python3_FOUND )
    execute_process( COMMAND
      ${Python3_EXECUTABLE} -c "import numpy ; print( numpy.get_include() );"
      OUTPUT_VARIABLE PYTHON_NUMPY_INCLUDE_PATH
      OUTPUT_STRIP_TRAILING_WHITESPACE )

    if ( PYTHON_NUMPY_INCLUDE_PATH )
      # uncomment following line to enable PYTHON_WRAPPERS build
      set ( RDK_BUILD_PYTHON_WRAPPERS "ON" )
      set ( PYTHON_NUMPY_INCLUDE_PATH ${PYTHON_NUMPY_INCLUDE_PATH} )
    endif()
endif()

###########################################################
####################### rdkit #############################
add_custom_target( rdkit
  COMMAND ${CMAKE_COMMAND}
  -DCURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
  -DRDBASE="${RDBASE}"
  -DRDKIT_SOURCE_DIR="${RDKIT_SOURCE_DIR}"
  -DRDKIT_BINARY_DIR="${RDKIT_BINARY_DIR}"
  -DRDKIT_REVISION="${RDKIT_REVISION}"
  -DRDK_BUILD_PYTHON_WRAPPERS="${RDK_BUILD_PYTHON_WRAPPERS}"
  -DPython3_EXECUTABLE="${Python3_EXECUTABLE}"
  -DPYTHON_NUMPY_INCLUDE_PATH="${PYTHON_NUMPY_INCLUDE_PATH}"
  -DGENERATOR="${GENERATOR}"
  -DEIGEN3_INCLUDE_DIR="${EIGEN_INSTALL_PREFIX}/include/eigen3"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/rdkit.cmake"
  COMMAND cmd /c "echo #################################################################################"
  COMMAND cmd /c "echo ## ${RDKIT_BINARY_DIR} has been created. ##"
  COMMAND cmd /c "cd ${RDKIT_BINARY_DIR} && bootstrap.bat"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR}
  )

message( STATUS "SOURCE_ROOT         = " ${SOURCE_ROOT} )
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
