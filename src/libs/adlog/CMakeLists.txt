project( adlog )
cmake_policy(SET CMP0022 NEW)

add_definitions(-DADLOG_LIBRARY)

add_library( adlog SHARED
	     adlog.cpp
	     logger.cpp
             logger.hpp
	     logging_handler.cpp
             logging_handler.hpp
             logging_debug.cpp
             logging_debug.hpp
             logging_file.cpp
             logging_file.hpp
             logging_syslog.cpp
             logging_syslog.hpp
	     )

target_include_directories( adlog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries( adlog LINK_PUBLIC
		       adportable
		       ${Boost_LIBRARIES}
		       )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib COMPONENT libraries )

install( FILES
  adlog_global.hpp
  logger.hpp
  logging_handler.hpp
  logging_debug.hpp
  logging_file.hpp
  logging_syslog.hpp
  DESTINATION include/adlog COMPONENT headers
  )
