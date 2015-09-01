#
# CMakeLists for adplugin shared library
#
project( adicontroller )

cmake_policy(SET CMP0028 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions(-DADICONTROLLER_LIBRARY)

add_library( ${PROJECT_NAME} SHARED
  adicontroller.cpp
  adicontroller_global.hpp
  eventlog.hpp
  eventlog.cpp
  instrument.hpp
  instrument.cpp
  manager.hpp
  manager.cpp
  octet_array.hpp
  octet_array.cpp
  receiver.hpp
  receiver.cpp
  signalobserver.hpp
  signalobserver.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC adplugin )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

