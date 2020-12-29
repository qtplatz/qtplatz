######

if ( NOT EXISTS "${GIT_CLONE_DIR}" )
  message( STATUS "git clone ${GIT_CLONE_URL} ${GIT_CLONE_DIR}" )
  execute_process( COMMAND git clone "${GIT_CLONE_URL}" "${GIT_CLONE_DIR}" )
endif()
