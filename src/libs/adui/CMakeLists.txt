cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Widgets )

project( adui )

add_definitions(-DADUI_LIBRARY)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${QtCore_INCLUDE_DIRS}
  ${QtWidgets_INCLUDE_DIRS}
  )

qt_add_resources(RC_SRC "adui.qrc")

add_library( ${PROJECT_NAME} SHARED
  manhattanstyle.cpp
  manhattanstyle.hpp
  styleanimator.cpp
  styleanimator.hpp
  ${RC_SRC}
  )

if ( ${QT_VERSION_MAJOR} GREATER 5 )
  set (UTILS "Utils")
else()
  set (UTILS "utils")
endif()

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  ${UTILS}
  Qt::Core
  Qt::Widgets
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
  ARCHIVE DESTINATION lib COMPONENT libraries )
