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

######
## Eigen3 install on Windows
##
set ( EIGEN_BUILD_DIR ${BUILD_ROOT}/eigen.build )
if ( NOT EXISTS ${EIGEN_BUILD_DIR} )
  file( MAKE_DIRECTORY ${EIGEN_BUILD_DIR} )
endif()

add_custom_command(
  OUTPUT ${EIGEN_SOURCE_DIR}
  COMMAND git clone https://github.com/eigenteam/eigen-git-mirror ${EIGEN_SOURCE_DIR}
  )

add_custom_command(
  OUTPUT ${EIGEN_BUILD_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${EIGEN_BUILD_DIR}
  )

add_custom_target( eigen
  DEPENDS ${EIGEN_SOURCE_DIR} ${EIGEN_BUILD_DIR}
  COMMAND cmake ${EIGEN_SOURCE_DIR}
  COMMAND cmake --build . --config Release
  COMMAND cmake --build . --target install
  WORKING_DIRECTORY ${EIGEN_BUILD_DIR}
  )
## End Eigen3 install.
######

######
## boost install on Windows
##
if ( NOT BOOST_VERSION )
  message( FATAL "Empty BOOST_VERSION" )
endif()
string ( REGEX REPLACE "([0-9]+)_([0-9]+)_([0-9]+)" "\\1" BOOST_Major ${BOOST_VERSION} )
string ( REGEX REPLACE "([0-9]+)_([0-9]+)_([0-9]+)" "\\2" BOOST_Minor ${BOOST_VERSION} )
string ( REGEX REPLACE "([0-9]+)_([0-9]+)_([0-9]+)" "\\3" BOOST_Micro ${BOOST_VERSION} )
message( STATUS "boost version (parsed) : " ${BOOST_Major}.${BOOST_Minor}.${BOOST_Micro} )

set ( BOOST_SOURCE_DIR ${BUILD_ROOT}/boost_${BOOST_VERSION} )
set ( BOOST_BUILD_DIR  ${BUILD_ROOT}/boost_${BOOST_VERSION} )

set ( BOOST_TARBALL "boost_${BOOST_VERSION}.tar.bz2" )
set ( BOOST_DOWNLOAD_URL "https://sourceforge.net/projects/boost/files/boost/${BOOST_Major}.${BOOST_Minor}.${BOOST_Micro}/${BOOST_TARBALL}/download" )

set ( BZIP2_TARBALL bzip2-1.0.6.tar.gz )
set ( BZIP2_SOURCE_DIR ${SOURCE_ROOT}/bzip2-1.0.6 )

add_custom_command(
  OUTPUT ${SOURCE_ROOT}/${BZIP2_TARBALL}
  COMMAND curl -L -o ${SOURCE_ROOT}/${BZIP2_TARBALL} "http://www.bzip.org/1.0.6/${BZIP2_TARBALL}"
  )

add_custom_command(
  OUTPUT ${SOURCE_ROOT}/${BOOST_TARBALL}
  COMMAND curl -L -o ${SOURCE_ROOT}/${BOOST_TARBALL} ${BOOST_DOWNLOAD_URL}
  )

# bzip2-1.0.6 -> ~/source/bzip2-1.0.6  (no compile/build required)
add_custom_command(
  OUTPUT ${BZIP2_SOURCE_DIR}
  DEPENDS ${SOURCE_ROOT}/${BZIP2_TARBALL}
  COMMAND ${CMAKE_COMMAND} -E tar xvf ${SOURCE_ROOT}/${BZIP2_TARBALL}
  WORKING_DIRECTORY ${SOURCE_ROOT}
  )

# boost_1_67_0 -> ~/source/build-vc15.0-x86_64/boost_1_67_0 (source in-tree build)
add_custom_command(
  OUTPUT ${BOOST_SOURCE_DIR}
  DEPENDS ${SOURCE_ROOT}/${BOOST_TARBALL}
  COMMAND ${CMAKE_COMMAND} -E tar xvf ${SOURCE_ROOT}/${BOOST_TARBALL}
  WORKING_DIRECTORY ${BUILD_ROOT}     # ~/source/build-vc15.0-x86_64  --> /boost_1.67_0/
  )

add_custom_command(
  OUTPUT ${BOOST_SOURCE_DIR}/b2.exe
  DEPENDS ${BOOST_SOURCE_DIR}
  COMMAND cmd /c bootstrap.bat
  WORKING_DIRECTORY ${BOOST_SOURCE_DIR}
  )

add_custom_target( boost
  DEPENDS ${BOOST_SOURCE_DIR}/b2.exe
  COMMAND b2 -j$ENV{NUMBER_OF_PROCESSORS} address-model=64 -s BZIP2_SOURCE=%BZIP2DIR% link=static --stagedir=stage/x64-static stage install
  COMMAND b2 -j$ENV{NUMBER_OF_PROCESSORS} address-model=64 -s BZIP2_SOURCE=%BZIP2DIR% link=shared --stagedir=stage/x64-shared stage install
  WORKING_DIRECTORY ${BOOST_SOURCE_DIR}
  )

## end boost install.
######

######
## RDKit install on Windows
##
set ( RDKIT_SOURCE_DIR ${RDBASE} )
set ( RDKIT_BUILD_DIR ${BUILD_ROOT}/rdkit.build )
if ( NOT EXISTS ${RDKIT_BUILD_DIR} )
  file( MAKE_DIRECTORY ${RDKIT_BUILD_DIR} )
endif()

add_custom_command(
  OUTPUT ${RDKIT_SOURCE_DIR}
  COMMAND git clone https://github.com/rdkit/rdkit ${RDKIT_SOURCE_DIR}
  )

add_custom_command(
  OUTPUT ${RDKIT_BUILD_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${RDKIT_BUILD_DIR}
  )

if ( $ENV{VisualStudioVersion} VERSION_LESS "15.0" )
  message( FATAL "Visual Studio Version $ENV{VisualStudioVersion} too old" )
else()
  set ( GENERATOR "Visual Studio 15 2017 Win64" )
endif()

find_package( Boost 1.57 REQUIRED COMPONENTS
  atomic
  bzip2
  chrono
  date_time
  filesystem
  iostreams
  locale
  program_options
  regex
  serialization
  system
  thread
  wserialization )

add_custom_target( rdkit
  DEPENDS ${RDKIT_SOURCE_DIR} ${RDKIT_BUILD_DIR}
  COMMAND cmake
  -DBOOST_LIBRARYDIR="C:/Boost/lib"
  -DBOOST_ROOT=${BOOST_ROOT}
  -DBoost_USE_STATIC_LIBS=ON
  -DRDK_BUILD_INCHI_SUPPORT=ON
  -DRDK_BUILD_PYTHON_WRAPPERS=OFF
  -DRDK_BUILD_SWIG_JAVA_WRAPPER=OFF
  -DRDK_INSTALL_STATIC_LIBS=ON
  -DRDK_INSTALL_DYNAMIC_LIBS=ON
  -DCMAKE_DEBUG_POSTFIX="d" -G ${GENERATOR} ${RDBASE} ${RDKIT_SOURCE_DIR}
  COMMAND cmake --build . --config Debug
  COMMAND cmake --build . --config Release
  COMMAND cmake --build . --target install
  WORKING_DIRECTORY ${RDKIT_BUILD_DIR}
  )

## end boost install.
######
