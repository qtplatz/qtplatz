project( xmlparser )

cmake_policy(SET CMP0022 NEW)

add_definitions(-DXMLPARSER_LIBRARY)
if ( WIN32 )
  add_definitions(-DPUGIXML_API=__declspec\(dllexport\))
endif()

add_library( ${PROJECT_NAME} SHARED
  pugiwrapper.cpp
  pugixml-1.12/src/pugixml.cpp )

target_include_directories( ${PROJECT_NAME} PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/pugixml-1.12/src"
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
  ARCHIVE DESTINATION lib COMPONENT libraries )
