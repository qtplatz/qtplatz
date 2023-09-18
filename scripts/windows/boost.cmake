######
## boost install on Windows
##
if ( NOT BOOST_VERSION )
  message( FATAL_ERROR "Empty BOOST_VERSION" )
endif()

if ( NOT DOWNLOADS )
  message( FATAL_ERROR "Empty DOWNLOADS" )
endif()

if ( NOT BOOST_SOURCE_DIR )
  message( FATAL_ERROR "Empty BOOST_SORUCE_DIR" )
endif()

if ( NOT BZIP2_SOURCE_DIR )
  message( FATAL_ERROR "Empty BZIP2_SORUCE_DIR" )
endif()

if ( NOT ZLIB_SOURCE_DIR )
  message( FATAL_ERROR "Empty ZLIB_SORUCE_DIR" )
endif()

if ( NOT ZLIB_INSTALL_PREFIX )
  message( FATAL_ERROR "Empty ZLIB_INSTALL_PREFIX" )
endif()

string ( REGEX REPLACE "([0-9]+)_([0-9]+)_([0-9]+)" "\\1" BOOST_Major ${BOOST_VERSION} )
string ( REGEX REPLACE "([0-9]+)_([0-9]+)_([0-9]+)" "\\2" BOOST_Minor ${BOOST_VERSION} )
string ( REGEX REPLACE "([0-9]+)_([0-9]+)_([0-9]+)" "\\3" BOOST_Micro ${BOOST_VERSION} )

set ( BOOST_DOT_VERSION "${BOOST_Major}.${BOOST_Minor}.${BOOST_Micro}" )

message( STATUS "boost version (parsed) : " ${BOOST_DOT_VERSION} )
message( STATUS "BOOST_SOURCE_DIR       : " ${BOOST_SOURCE_DIR} )
message( STATUS "BZIP2_SOURCE_DIR       : " ${BZIP2_SOURCE_DIR} )

file ( TO_NATIVE_PATH ${BZIP2_SOURCE_DIR} BZIP2_SOURCE_PATH ) # use in boost-build.bat.in
file ( TO_NATIVE_PATH ${ZLIB_SOURCE_DIR} ZLIB_SOURCE_PATH ) # use in boost-build.bat.in
file ( TO_NATIVE_PATH ${ZLIB_BINARY_DIR} ZLIB_BUILD_PATH ) # use in boost-build.bat.in

set ( BOOST_TARBALL "boost_${BOOST_VERSION}.tar.bz2" )
set ( BOOST_DOWNLOAD_URL "https://boostorg.jfrog.io/artifactory/main/release/${BOOST_DOT_VERSION}/source/${BOOST_TARBALL}" )

set ( BZIP2_TARBALL "bzip2-1.0.6.tar.gz" )
set ( BZIP2_DOWNLOAD_URL "https://sourceforge.net/projects/bzip2/files/latest/download" )

set ( ZLIB_TARBALL "zlib-1.3.tar.gz" )
set ( ZLIB_DOWNLOAD_URL "https://zlib.net/${ZLIB_TARBALL}" )

get_filename_component( __boost_parent ${BOOST_SOURCE_DIR} DIRECTORY )
get_filename_component( __bzip2_parent ${BZIP2_SOURCE_DIR} DIRECTORY )
get_filename_component( __zlib_parent ${ZLIB_SOURCE_DIR} DIRECTORY )

if ( NOT EXISTS ${DOWNLOADS}/${BOOST_TARBALL} )
  message( STATUS "------- DOWNLOAD ${BOOST_DOWNLOAD_URL} ${DOWNLOADS}/${BOOST_TARBALL}" )
  file( DOWNLOAD ${BOOST_DOWNLOAD_URL} ${DOWNLOADS}/${BOOST_TARBALL} SHOW_PROGRESS STATUS BOOST_DL_STATUS)
  message( STATUS "##### BOOST_DL_STATUS: " ${BOOST_DL_STATUS} )
else()
  message( STATUS "------- ${DOWNLOADS}/${BOOST_TARBALL} --- exists." )
endif()

if ( NOT EXISTS ${DOWNLOADS}/${BZIP2_TARBALL} )
  file( DOWNLOAD ${BZIP2_DOWNLOAD_URL} ${DOWNLOADS}/${BZIP2_TARBALL} SHOW_PROGRESS )
endif()

if ( NOT EXISTS ${DOWNLOADS}/${ZLIB_TARBALL} )
  file( DOWNLOAD ${ZLIB_DOWNLOAD_URL} ${DOWNLOADS}/${ZLIB_TARBALL} SHOW_PROGRESS )
endif()

if ( NOT TAR )
  message( STATUS "================= No tar command specified --> " ${TAR})
  set ( TAR tar )
endif()

message( STATUS "------------------> working directory for boost: " ${__boost_parent} )
message( STATUS "------------------> working directory for bzip2: " ${__bzip2_parent} )
message( STATUS "------------------> working directory for zlib:  " ${__zlib_parent} )

if ( NOT EXISTS ${BOOST_SOURCE_DIR}/boost )
  message( STATUS "${TAR} xvf ${DOWNLOADS}/${BOOST_TARBALL} -C ${__boost_parent}" )
  execute_process( COMMAND ${CMAKE_COMMAND} -E ${TAR} xvf ${DOWNLOADS}/${BOOST_TARBALL} WORKING_DIRECTORY ${__boost_parent} )
endif()

if ( NOT EXISTS ${BZIP2_SOURCE_DIR} )
  message( STATUS "${TAR} xvf ${DOWNLOADS}/${BZIP2_TARBALL} -C ${__bzip2_parent}" )
  execute_process( COMMAND ${CMAKE_COMMAND} -E ${TAR} xvf ${DOWNLOADS}/${BZIP2_TARBALL} WORKING_DIRECTORY ${__bzip2_parent} )
endif()

if ( NOT EXISTS ${ZLIB_SOURCE_DIR} )
  message( STATUS "${TAR} xvf ${DOWNLOADS}/${ZLIB_TARBALL} -C ${__zlib_parent}" )
  execute_process( COMMAND ${CMAKE_COMMAND} -E ${TAR} xvf ${DOWNLOADS}/${ZLIB_TARBALL} WORKING_DIRECTORY ${__zlib_parent} )
endif()

configure_file(
  ${CURRENT_SOURCE_DIR}/boost-build.bat.in
  ${BOOST_SOURCE_DIR}/boost-build.bat
  )
