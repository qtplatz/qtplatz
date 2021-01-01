######

if ( NOT DOWNLOADS )
  if ( NOT $ENV{HOME} STREQUAL "")
    set ( DOWNLOADS $ENV{HOME}/Downloads )
  elseif ( NOT $ENV{USERPROFILE} STREQUAL "")
    set ( DOWNLOADS $ENV{USERPROFILE}/Downloads )
  else()
    message( FATAL_ERROR "No user home directory found" )
  endif()
endif()

if ( NOT DOWNLOAD_URL )
  message( FATAL_ERROR "No DOWNLOAD_URL specified" )
endif()

if ( NOT DOWNLOAD_FILE )
  message( FATAL_ERROR "No DOWNLOAD_FILE specified" )
endif()

if ( NOT EXISTS ${DOWNLOADS}/${DOWNLOAD_FILE} )
  message( STATUS "downloading ${DOWNLOADS}/${DOWNLOAD_FILE}" )
  file( DOWNLOAD ${DOWNLOAD_URL} ${DOWNLOADS}/${DOWNLOAD_FILE} SHOW_PROGRESS )
endif()

if ( NOT TAR )
  set ( TAR tar )
endif()

if ( NOT EXISTS "${DOWNLOAD_SOURCE_DIR}" AND EXISTS "${DOWNLOADS}/${DOWNLOAD_FILE}" )
  message( STATUS "download source dest: " "${DOWNLOAD_SOURCE_DIR}" )
  get_filename_component( __parent_dir ${DOWNLOAD_SOURCE_DIR} DIRECTORY )
  message( STATUS "tar xvf ${DOWNLOADS}/${DOWNLOAD_FILE} at " "${__parent_dir}" )
  execute_process( COMMAND "${CMAKE_COMMAND}" -E ${TAR} xvf "${DOWNLOADS}/${DOWNLOAD_FILE}" WORKING_DIRECTORY "${__parent_dir}" )
endif()
