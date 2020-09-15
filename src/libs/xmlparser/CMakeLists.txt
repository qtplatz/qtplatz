project( xmlparser )

cmake_policy(SET CMP0022 NEW)

add_definitions(-DXMLPARSER_LIBRARY)
if ( WIN32 )
  add_definitions(-DPUGIXML_API=__declspec\(dllexport\))
endif()

add_library( ${PROJECT_NAME} SHARED
  pugiwrapper.cpp pugixml.cpp )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib COMPONENT libraries
  )
