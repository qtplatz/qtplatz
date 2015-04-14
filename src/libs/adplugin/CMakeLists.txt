#
# CMakeLists for adplugin shared library
#
project( adplugin )

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

find_package( Boost 1.57 COMPONENTS date_time system regex )

find_package( Qt5 5.3.2 CONFIG REQUIRED Core )

include_directories( ${Qt5Core_INCLUDE_DIRS}  )

if ( NOT Qt5Core_FOUND )
  message( FATAL_ERROR "Qt5Core: " ${Qt5Core} )
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions(-DADPLUGIN_LIBRARY)

add_library( adplugin SHARED
    lifecycle.cpp
    manager.cpp
    loader.cpp
    plugin.cpp
    orbfactory.cpp
    orbbroker.cpp
    orbservant.cpp
    plugin_ptr.cpp
    visitor.cpp
    lifecycleaccessor.cpp
)

#target_include_directories( adcontrols PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries( adplugin LINK_PUBLIC
  ${Boost_LIBRARIES}
  adportable
  qtwrapper
  xmlparser
  adlog
  Qt5::Core )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
