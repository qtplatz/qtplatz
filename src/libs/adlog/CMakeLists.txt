project( adlog )
cmake_policy(SET CMP0022 NEW)

add_definitions(-DADLOG_LIBRARY)

add_library( adlog SHARED
	     adlog.cpp
	     logger.cpp
	     logging_handler.cpp
	     )

target_include_directories( adlog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries( adlog LINK_PUBLIC
		       adportable
		       ${Boost_LIBRARIES}
		       )

#set_target_properties( adlog
#  PROPERTIES
#  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_BINARY_DIR}/lib/qtplatz )    
		     
install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules    
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
