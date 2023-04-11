project( acewrapper )

if (WIN32)
  add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS -wd4996 )
endif()

add_definitions(-DACEWRAPPER_LIBRARY)

add_library( ${PROJECT_NAME} SHARED
  constants.cpp
  input_buffer.cpp
  ifconfig.cpp
  ifconfig_windows.cpp
  udpeventreceiver.cpp
  udpeventreceiver.hpp
  udpeventsender.cpp
  udpeventsender.hpp
  )

target_link_libraries(
  ${PROJECT_NAME} LINK_PUBLIC
  adportable
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
