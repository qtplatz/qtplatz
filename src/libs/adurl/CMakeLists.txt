cmake_minimum_required (VERSION 2.8.11)
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
  sse2.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adio
  adportable
  Threads::Threads
  ${Boost_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries 
  ARCHIVE DESTINATION lib COMPONENT libraries )

install( FILES
  adurl_global.h
  ajax.hpp
  blob.hpp
  DESTINATION include/adurl COMPONENT headers )
