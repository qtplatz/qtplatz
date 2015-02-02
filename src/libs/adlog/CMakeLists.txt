project( adlog )
cmake_policy(SET CMP0022 NEW)

#set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.56 COMPONENTS date_time system filesystem )

add_definitions(-DADLOG_LIBRARY)

add_library( adlog SHARED
	     adlog.cpp
	     logger.cpp
	     logging_handler.cpp
	     )

target_include_directories( adlog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries( adlog LINK_PUBLIC
		       ${Boost_LIBRARIES}
		       adportable
		       )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
