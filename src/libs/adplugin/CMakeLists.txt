#
# CMakeLists for adplugin shared library
#
project( adplugin )

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

find_package( Boost 1.57 COMPONENTS date_time system regex )

if ( Qt5_FOUND )
  find_package( Qt5 COMPONENTS Core )
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions(-DADPLUGIN_LIBRARY)

if ( Qt5Core_FOUND )

  add_library( adplugin_manager SHARED
    manager.cpp
    loader.cpp
    lifecycleaccessor.cpp
    )

  include_directories( ${Qt5Core_INCLUDE_DIRS}  )

  target_link_libraries( adplugin_manager LINK_PUBLIC
    ${Boost_LIBRARIES}
    adportable
    xmlparser
    adlog
    Qt5::Core )

  install( TARGETS adplugin_manager
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib/qtplatz COMPONENT modules
    ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
  
endif()


add_library( adplugin SHARED
    lifecycle.cpp
    plugin.cpp
    orbfactory.cpp
    orbbroker.cpp
    orbservant.cpp
    plugin_ptr.cpp
    visitor.cpp
    )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  ${Boost_LIBRARIES}
  adportable
  xmlparser
  adlog )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

