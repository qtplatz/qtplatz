cmake_minimum_required ( VERSION 3.14 )

project ( ws_adprocessor )

#include_directories( ${LIBRARYLINK_INCLUDE_DIRS} ${WSTP_INCLUDE_DIRS} )
include_directories( ${WSTP_INCLUDE_DIRS} )
link_directories( ${WSTP_LIBRARY_DIRS} )

link_directories(
  ${WSTP_LIBRARY_DIRS}
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

WSTP_ADD_TM(adprocessor.tm .cxx)

add_executable( ${PROJECT_NAME}
  adprocessor.cpp
  adprocessor.tm.cxx
  dataprocessor.cpp
  dataprocessor.hpp
  datareader.cpp
  datareader.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adfs
  adplugin_manager
  adprocessor
  acqrscontrols
  adportable
  ${Boost_LIBRARIES}
  ${WSTP_LIBRARIES}
  )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  SUFFIX ".bin"
  RUNTIME_OUTPUT_DIRECTORY ${WSTP_MODULE_DIR}
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${WSTP_MODULE_DIR}
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${WSTP_MODULE_DIR}  
  LIBRARY_OUTPUT_DIRECTORY ${WSTP_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY_DEBUG ${WSTP_MODULE_DIR}
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${WSTP_MODULE_DIR}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${WSTP_DIST_PACKAGES}" COMPONENT modules
  LIBRARY DESTINATION "${WSTP_DIST_PACKAGES}" COMPONENT modules
  )
