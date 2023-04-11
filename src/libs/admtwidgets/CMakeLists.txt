cmake_minimum_required (VERSION 3.18.2)
cmake_policy( SET CMP0022 NEW )
project ( admtwidgets)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#find_package( Qt${QT_VERSION_MAJOR} REQUIRED Core Gui PrintSupport Sql Svg Widgets )
find_package( Threads REQUIRED )

include_directories(
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/contrib/multum/libs
  ${CMAKE_SOURCE_DIR}/contrib/include
  ${QWT_DIR}/include
  ${QtWidgets_INCLUDE_DIRS}
  )

add_definitions( -DADMTWIDGETS_LIBRARY )

if ( WIN32 )
  add_definitions( -wd4996 )
endif()

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

add_library( ${PROJECT_NAME} SHARED
  admtwidgets_global.hpp
  protocolform.cpp
  protocolform.hpp
  protocolwidget.cpp
  protocolwidget.hpp
  moltablewidget.cpp
  moltablewidget.hpp
)

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  admtcontrols
  adwidgets
  adcontrols
  adportable
  adlog
  adplugin
  adurl
  adio
  qtwrapper
  Qt::Sql
  Qt::Svg
  Qt::Widgets
  ${Boost_LIBRARIES}
  Threads::Threads
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
