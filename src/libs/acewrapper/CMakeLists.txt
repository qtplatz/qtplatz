project( acewrapper )

if (WIN32)
  add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -wd4996 )
endif()

add_library( acewrapper
  constants.cpp
  input_buffer.cpp
  ifconfig.cpp
  ifconfig_windows.cpp
  iorsender.cpp
  iorquery.cpp
  udpeventreceiver.cpp
  udpeventreceiver.hpp
  udpeventsender.cpp
  udpeventsender.hpp
)
           
install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib COMPONENT libraries
  )
