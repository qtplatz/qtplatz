project( xmlparser )

cmake_policy(SET CMP0022 NEW)

add_library( xmlparser
  pugiwrapper.cpp pugixml.cpp )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
