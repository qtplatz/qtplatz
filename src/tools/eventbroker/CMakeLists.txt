cmake_minimum_required (VERSION 3.18.2)

project (eventbroker)

cmake_policy(SET CMP0022 NEW)

include_directories( ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/libs )

link_directories( ${Boost_LIBRARY_DIRS} ${CMAKE_BINARY_DIR}/lib )

add_definitions( -DEVENTBROKER_LIBRARY )

#if ( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
#  set( _libs pthread )
#endif()

add_library( eventbroker SHARED
  api.cpp
  document.cpp
  )

target_link_libraries( eventbroker LINK_PUBLIC
  adportable
  acewrapper
  ${Boost_LIBRARIES}
  Threads::Threads #${_libs}
  )

runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
  LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
  ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
  )

install( FILES
  eventbroker.h
  DESTINATION include/eventbroker COMPONENT headers )
