cmake_minimum_required (VERSION 2.8.11)
cmake_policy( SET CMP0022 NEW )

project( adurl )

find_package( Boost 1.59 REQUIRED COMPONENTS system program_options date_time regex )

if ( NOT WIN32 )
  set( oslibs "pthread" )
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

add_definitions( -DADURL_LIBRARY )

add_executable( adurltool main.cpp )
add_library( adurl SHARED client.cpp client.hpp request.cpp request.hpp dg.cpp dg.hpp sse.hpp sse.cpp )

target_link_libraries( adurl LINK_PUBLIC
  adportable
  ${oslibs}
  ${Boost_LIBRARIES}
  )

target_link_libraries( adurltool LINK_PUBLIC
  adurl
  adportable
  ${oslibs}
  ${Boost_LIBRARIES}
  )
  
install( TARGETS adurltool RUNTIME DESTINATION bin COMPONENT applications )

install( TARGETS adurl 
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries 
  ARCHIVE DESTINATION lib COMPONENT libraries )
