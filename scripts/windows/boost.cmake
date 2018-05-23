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

string ( REGEX REPLACE "([0-9]+)_([0-9]+)_([0-9]+)" "\\1" BOOST_Major ${BOOST_VERSION} )
string ( REGEX REPLACE "([0-9]+)_([0-9]+)_([0-9]+)" "\\2" BOOST_Minor ${BOOST_VERSION} )
string ( REGEX REPLACE "([0-9]+)_([0-9]+)_([0-9]+)" "\\3" BOOST_Micro ${BOOST_VERSION} )

message( STATUS "boost version (parsed) : " ${BOOST_Major}.${BOOST_Minor}.${BOOST_Micro} )
message( STATUS "BOOST_SOURCE_DIR       : " ${BOOST_SOURCE_DIR} )
message( STATUS "BZIP2_SOURCE_DIR       : " ${BZIP2_SOURCE_DIR} )

set ( BOOST_TARBALL "boost_${BOOST_VERSION}.tar.bz2" )
set ( BOOST_DOWNLOAD_URL "https://sourceforge.net/projects/boost/files/boost/${BOOST_Major}.${BOOST_Minor}.${BOOST_Micro}/${BOOST_TARBALL}/download" )

set ( BZIP2_TARBALL bzip2-1.0.6.tar.gz )
set ( BZIP2_DOWNLOAD_URL "http://www.bzip.org/1.0.6/${BZIP2_TARBALL}" )

get_filename_component( __boost_parent ${BOOST_SOURCE_DIR} DIRECTORY )
message( STATUS "tar xvf ${DOWNLOADS}/${BOOST_TARBALL} -C ${__boost_parent}" )
get_filename_component( __bzip2_parent ${BZIP2_SOURCE_DIR} DIRECTORY )
message( STATUS "tar xvf ${DOWNLOADS}/${BZIP2_TARBALL} -C ${__bzip2_parent}" )

if ( NOT EXISTS ${DOWNLOADS}/${BOOST_TARBALL} )
  file( DOWNLOAD ${BOOST_DOWNLOAD_URL} ${DOWNLOADS}/${BOOST_TARBALL} SHOW_PROGRESS )
endif()

if ( NOT EXISTS ${DOWNLOADS}/${BZIP2_TARBALL} )
  file( DOWNLOAD ${BZIP2_DOWNLOAD_URL} ${DOWNLOADS}/${BZIP2_TARBALL} SHOW_PROGRESS )
endif()

if ( NOT EXISTS ${BOOST_SOURCE_DIR}/bootstrap.bat )
  message( STATUS "boost source dir ${BOOST_SOURCE_DIR} not exit" )
  execute_process( COMMAND ${CMAKE_COMMAND} -E tar xvf ${DOWNLOADS}/${BOOST_TARBALL} -C ${__boost_parent} )
else()
  message( STATUS "boost source dir ${BOOST_SOURCE_DIR} exit" )
endif()

if ( NOT EXISTS ${BZIP2_SOURCE_DIR} )
  execute_process( COMMAND ${CMAKE_COMMAND} -E tar xvf ${DOWNLOADS}/${BZIP2_TARBALL} -C ${__bzip2_parent} )
endif()

