#
# CMakeLists for adplugin shared library
#
project( adplugin_manager )

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

find_package( Boost 1.57 COMPONENTS date_time system regex )
find_package( Qt5 COMPONENTS Core REQUIRED )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions(-DADPLUGIN_LIBRARY)

add_library( ${PROJECT_NAME} SHARED
  manager.cpp
  loader.cpp
  lifecycleaccessor.cpp
  )

include_directories( ${Qt5Core_INCLUDE_DIRS}  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  ${Boost_LIBRARIES}
  adportable
  xmlparser
  adlog
  adplugin
  Qt5::Core )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

