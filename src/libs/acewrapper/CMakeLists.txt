project( acewrapper )

if (WIN32)
  add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -wd4996 )
endif()

add_library( ${PROJECT_NAME} SHARED
  constants.cpp
  input_buffer.cpp
  ifconfig.cpp
  ifconfig_windows.cpp
#  iorsender.cpp
#  iorquery.cpp
  udpeventreceiver.cpp
  udpeventreceiver.hpp
  udpeventsender.cpp
  udpeventsender.hpp
  )

target_link_libraries(
  ${PROJECT_NAME} LINK_PUBLIC
  adportable
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
  ARCHIVE DESTINATION lib COMPONENT libraries )
