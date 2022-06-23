
project(aqmd3widgets)

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#find_package( Qt${QT_VERSION_MAJOR} REQUIRED PrintSupport Sql Svg Widgets Gui Core )

include_directories(
  ${QWT_INCLUDE_DIR}
  ${QtPrintSupport_INCLUDE_DIRS}
  ${QtWidgets_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/..
  )

add_definitions(-DAQMD3WIDGETS_LIBRARY)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set( ${PROJECT_NAME}_SOURCES
  thresholdwidget.cpp
  thresholdwidget.hpp
  aqmd3form.cpp
  aqmd3form.hpp
  aqmd3table.cpp
  aqmd3table.hpp
  aqmd3widget.cpp
  aqmd3widget.hpp
)

add_library( ${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS} )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  aqmd3controls
  adplugin
  adplugin_manager
  adlog
  adcontrols
  adportable
  adwidgets
  qtwrapper
  ${QWT_LIBRARIES}
  Qt::Widgets
  Qt::PrintSupport
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )
