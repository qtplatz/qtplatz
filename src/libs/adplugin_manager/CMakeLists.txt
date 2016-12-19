#
# CMakeLists for adplugin shared library
#
project( adplugin_manager )

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

if ( NOT Boost_FOUND OR WIN32 )
  find_package( Boost 1.57 COMPONENTS date_time system regex )
endif()
find_package( Qt5 COMPONENTS Core REQUIRED )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_definitions(-DADPLUGIN_LIBRARY)

add_library( ${PROJECT_NAME} SHARED
  manager.cpp
  manager.hpp
  loader.cpp
  loader.hpp
  lifecycleaccessor.cpp
  lifecycleaccessor.hpp
  lifecycle.hpp
  )

include_directories( ${Qt5Core_INCLUDE_DIRS}  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adportable
  xmlparser
  adlog
  adplugin
  Qt5::Core
  ${Boost_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

