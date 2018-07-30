
project(acqrswidgets)

cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package( Qt5 REQUIRED PrintSupport Sql Svg Widgets Gui Core )

include_directories(
  ${QWT_INCLUDE_DIR}
  ${Qt5PrintSupport_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/..
  )

add_definitions(-DACQRSWIDGETS_LIBRARY)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set( ${PROJECT_NAME}_SOURCES
  ap240form.cpp
  ap240form.hpp
  ap240horizontalform.cpp
  ap240horizontalform.hpp
  ap240triggerform.cpp
  ap240triggerform.hpp
  ap240verticalform.cpp
  ap240verticalform.hpp
  ap240widget.cpp
  ap240widget.hpp
  acqiriswidget.cpp
  acqiriswidget.hpp
  thresholdwidget.cpp
  thresholdwidget.hpp
  u5303aform.cpp
  u5303aform.hpp
  u5303atable.cpp
  u5303atable.hpp
  u5303awidget.cpp
  u5303awidget.hpp
)

add_library( ${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS} )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  acqrscontrols
  adplugin
  adplugin_manager
  adlog
  adcontrols
  adportable
  adwidgets
  qtwrapper
  ${QWT_LIBRARIES}
  Qt5::Widgets
  Qt5::PrintSupport
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )

install(
  FILES
  acqrswidgets_global.hpp  
  ap240form.hpp
  ap240horizontalform.hpp
  ap240triggerform.hpp
  ap240verticalform.hpp
  ap240widget.hpp
  acqiriswidget.hpp
  thresholdwidget.hpp
  u5303aform.hpp
  u5303atable.hpp
  u5303awidget.hpp
  DESTINATION include/acqrswidgets COMPONENT headers
  )
