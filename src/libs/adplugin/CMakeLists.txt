#
# CMakeLists for adplugin shared library
#
project( adplugin )

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

#find_package( Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED )
include_directories( ${QtCore_INCLUDE_DIRS}  )

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

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib COMPONENT libraries
)
