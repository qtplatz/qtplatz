project( xmlparser )

cmake_policy(SET CMP0022 NEW)

add_definitions(-DXMLPARSER_LIBRARY)
if ( WIN32 )
  add_definitions(-DPUGIXML_API=__declspec\(dllexport\))
endif()

add_library( ${PROJECT_NAME} SHARED
  pugiwrapper.cpp
  pugixml-1.15/src/pugixml.cpp
)

target_include_directories( ${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pugixml-1.15/src>"
  "$<INSTALL_INTERFACE:include/pugixml-1.15/src>"
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
