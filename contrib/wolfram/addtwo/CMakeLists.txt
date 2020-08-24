
project( addtwo )

WSTP_ADD_TM(addtwo.tm .cxx)

include_directories( ${WSTP_INCLUDE_DIRS} )
link_directories( ${WSTP_LIBRARY_DIRS} )

add_executable ( ${PROJECT_NAME} addtwo.tm.cxx addtwo.cxx )

target_link_libraries( ${PROJECT_NAME}
  ${WSTP_LIBRARIES}
  )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  #  PREFIX ""
  SUFFIX ".bin"
  RUNTIME_OUTPUT_DIRECTORY ${WSTP_MODULE_DIR}
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${WSTP_MODULE_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${WSTP_MODULE_DIR}  
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${WSTP_DIST_PACKAGES}" COMPONENT modules
  LIBRARY DESTINATION "${WSTP_DIST_PACKAGES}" COMPONENT modules
  )
