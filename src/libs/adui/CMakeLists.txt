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

if ( ${IDE_VERSION_MAJOR} GREATER_EQUAL 8 )
  set (UTILS "Utils")
else()
  set (UTILS "utils")
endif()

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  ${UTILS}
  Qt::Core
  Qt::Widgets
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

set_property( TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)  # for utils/algorithms.h

##########
