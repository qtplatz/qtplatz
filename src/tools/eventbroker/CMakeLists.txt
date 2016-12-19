cmake_minimum_required (VERSION 2.8.11)

project (eventbroker)

cmake_policy(SET CMP0022 NEW)

find_package( Boost 1.57 REQUIRED COMPONENTS system )

include_directories( ${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/libs )

link_directories( ${Boost_LIBRARY_DIRS} ${CMAKE_BINARY_DIR}/lib )

add_definitions( -DEVENTBROKER_LIBRARY )

if ( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
  set( _libs pthread )
endif()

add_library( eventbroker SHARED
  api.cpp 
  document.cpp 
  )

target_link_libraries( eventbroker LINK_PUBLIC
		       adportable
		       acewrapper
		       ${Boost_LIBRARIES}
		       ${_libs}
		       )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
