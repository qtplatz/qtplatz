cmake_minimum_required( VERSION 3.11 )

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
set ( EIGEN_SOURCE_DIR ${BUILD_ROOT}/eigen )
set ( EIGEN_BINARY_DIR ${BUILD_ROOT}/eigen.build )

set ( RDBASE ${SOURCE_ROOT}/rdkit )  # <- RDKit source & install directory
set ( RDKIT_BINARY_DIR "${BUILD_ROOT}/rdkit" )
set ( RDKIT_SOURCE_DIR "${RDBASE}" )
set ( NUMBER_OF_PROCESSORS $ENV{NUMBER_OF_PROCESSORS} )

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
  message( STATUS "QTDIR               = " ${QTDIR} )
endif()

###########################################################
### qwt

include ( qwt.cmake )

###################### boost #########################
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
  COMMAND cmd /c "echo ####################################################"
  COMMAND cmd /c "echo ################# boost.bat done ###################"
  COMMAND cmd /c "echo ####################################################"
  COMMAND cmd /c "echo run eigen-build.bat in the direcoty ${EIGEN_BINARY_DIR}"
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

###################### eigen #########################
add_custom_target( eigen
  COMMAND ${CMAKE_COMMAND}
  -DEIGEN_SOURCE_DIR=${EIGEN_SOURCE_DIR}
  -DEIGEN_BINARY_DIR=${EIGEN_BINARY_DIR}
  -DGENERATOR="NMake Makefiles"
  -P "${CMAKE_CURRENT_SOURCE_DIR}/eigen.cmake"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR}
  COMMAND cmd /c "eigen.bat"
  COMMAND cmd /c "echo ####################################################"
  COMMAND cmd /c "echo ################# eigen.bat done ###################"
  COMMAND cmd /c "echo ####################################################"
  COMMAND cmd /c "echo run nmake install in the direcoty ${EIGEN_BINARY_DIR} (require administrator privilidge)"
  )

set ( RDK_BUILD_PYTHON_WRAPPERS "OFF" )
find_package( Python3 )
if ( Python3_FOUND )
  #-DPYTHON_NUMPY_INCLUDE_PATH="$(python -c 'import numpy ; print(numpy.get_include())')" \
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
  -DRDBASE=${RDBASE}
  -DRDKIT_SOURCE_DIR=${RDKIT_SOURCE_DIR}
  -DRDKIT_BINARY_DIR=${RDKIT_BINARY_DIR}
  -DRDKIT_REVISION=${RDKIT_REVISION}
  -DRDK_BUILD_PYTHON_WRAPPERS=${RDK_BUILD_PYTHON_WRAPPERS}
  -DGENERATOR=${GENERATOR}
  -P "${CMAKE_CURRENT_SOURCE_DIR}/rdkit.cmake"
  COMMENT "Run rdkit.bat"
  WORKING_DIRECTORY ${CMAKE_CURRENT_BUILD_DIR}
  COMMAND cmd /c "rdkit.bat"
  COMMAND cmd /c "echo ####################################################"
  COMMAND cmd /c "echo ################# rdkit.bat done ###################"
  COMMAND cmd /c "echo ####################################################"
  COMMAND cmd /c "echo if you failed with zlib.h not found error, you may coment out zlib.h inclusion in GraphMol/FileParsers/PNGParser.cpp"
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
message( STATUS "Python3_FOUND       = " ${Python3_FOUND} )
if ( Python3_FOUND )
  message( STATUS "\tEXECUTABLE     = " ${Python3_EXECUTABLE} )
  message( STATUS "\tNUMPY_INCLUDE  = " ${PYTHON_NUMPY_INCLUDE_PATH} )
endif()
