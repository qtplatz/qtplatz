#
# CMakeLists for adplugin shared library
#
project( adplugin )

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

if ( NOT Boost_FOUND )
  find_package( Boost 1.57 COMPONENTS date_time system regex )
endif()

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
#  orbfactory.cpp
#  orbfactory.hpp
#  orbbroker.cpp
#  orbbroker.hpp
#  orbservant.cpp
#  orbservant.hpp
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
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

