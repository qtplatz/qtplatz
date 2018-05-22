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

