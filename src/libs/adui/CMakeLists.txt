cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

find_package( Qt5 CONFIG REQUIRED Widgets )

project( adui )

add_definitions(-DADUI_LIBRARY)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  )

qt_add_resources(RC_SRC "adui.qrc")

add_library( ${PROJECT_NAME} SHARED
  manhattanstyle.cpp
  manhattanstyle.hpp
  styleanimator.cpp
  styleanimator.hpp
  ${RC_SRC}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  utils
  Qt5::Core
  Qt5::Widgets
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
  ARCHIVE DESTINATION lib COMPONENT libraries )