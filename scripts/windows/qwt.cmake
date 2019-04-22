######
## QWT install on Windows

if ( NOT DOWNLOADS )
  message( FATAL_ERROR "Empty DOWNLOADS" )
endif()
if ( NOT QWT_SOURCE_DIR )
  message( FATAL_ERROR "Empty QWT_SOURCE_DIR" )
endif()

set ( QWT_BUILD_DIR ${QWT_SOURCE_DIR} )
set ( QWT_TARBALL "qwt-6.1.3.tar.bz2" )
set ( QWT_DOWNLOAD_URL "https://sourceforge.net/projects/qwt/files/qwt/6.1.3/qwt-6.1.3.tar.bz2/download" )

if ( NOT EXISTS ${DOWNLOADS}/${QWT_TARBALL} )
  file( DOWNLOAD ${QWT_DOWNLOAD_URL} ${DOWNLOADS}/${QWT_TARBALL} SHOW_PROGRESS )
endif()

if ( NOT EXISTS ${QWT_SOURCE_DIR} )
  get_filename_component( __qwt_parent ${QWT_SOURCE_DIR} DIRECTORY )
  message( STATUS "tar xvf ${DOWNLOADS}/${QWT_TARBALL} -C ${__qwt_parent}" )
  execute_process( COMMAND ${CMAKE_COMMAND} -E tar xvf ${DOWNLOADS}/${QWT_TARBALL} -C ${__qwt_parent} )
endif()

add_custom_command(
  OUTPUT ${QWT_SOURCE_DIR}/qwtconfig.pri.orig
  COMMAND ${CMAKE_COMMAND} -E rename ${QWT_SOURCE_DIR}/qwtconfig.pri ${QWT_SOURCE_DIR}/qwtconfig.pri.orig
  WORKING_DIRECTORY ${BUILD_ROOT}
  )

add_custom_target( qwtconfig
  DEPENDS ${QWT_SOURCE_DIR}
  DEPENDS ${QWT_SOURCE_DIR}/qwtconfig.pri.orig
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/qwtconfig.cmake
  WORKING_DIRECTORY ${QWT_SOURCE_DIR}    
  )

add_custom_target( qwt
  DEPENDS ${QWT_SOURCE_DIR}
  DEPENDS ${QWT_SOURCE_DIR}/qwtconfig.pri.orig
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/qwtconfig.cmake
  COMMAND ${QMAKE} qwt.pro
  COMMAND nmake
  COMMAND nmake install
  WORKING_DIRECTORY ${QWT_SOURCE_DIR}  
  )

######
