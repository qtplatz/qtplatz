#
# CMakeLists for adplugin shared library
#
project( adplugin )

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions(-DADPLUGIN_LIBRARY)

add_library( adplugin SHARED
  lifecycle.cpp
  lifecycle.hpp
  plugin.cpp
  plugin.hpp
  plugin_ptr.cpp
  plugin_ptr.hpp
  visitor.cpp
  visitor.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  xmlparser
  adlog
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
