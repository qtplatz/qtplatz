cmake_minimum_required (VERSION 3.18.2)
cmake_policy( SET CMP0022 NEW )

project( adurl )

find_package( Threads REQUIRED )

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

add_definitions( -DADURL_LIBRARY )

add_library( adurl SHARED
  ajax.cpp
  ajax.hpp
  blob.cpp
  blob.hpp
  client.cpp
  client.hpp
  request.cpp
  request.hpp
  dg.cpp
  dg.hpp
  sse.hpp
  sse.cpp
  #  sse2.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adio
  adportable
  Threads::Threads
  ${Boost_LIBRARIES}
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
  adurl_global.h
  ajax.hpp
  blob.hpp
  DESTINATION include/adurl COMPONENT headers )
