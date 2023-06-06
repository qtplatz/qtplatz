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

if ( WIN32 )
  link_directories(${BOOST_LIBRARYDIR})
  list( APPEND Boost_LIBRARIES "${BOOST_LIBRARYDIR}/boost_log-vc143-mt-x64-1_79.lib")
endif()

target_include_directories( adlog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

message( STATUS "==adlog== ${Boost_log_LIBRARY}")

target_link_libraries(
  adlog LINK_PUBLIC
  adportable
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
  adlog_global.hpp
  logger.hpp
  logging_handler.hpp
  logging_debug.hpp
  logging_file.hpp
  logging_syslog.hpp
  DESTINATION include/adlog COMPONENT headers
  )
