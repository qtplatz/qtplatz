cmake_minimum_required( VERSION 3.5 )
project( surface )
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0028 NEW)

set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )

set ( STANDALONE ON CACHE BOOL "STANDALONE" )
if ( STANDALONE )
  include( "standalone.cmake" )
endif()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

find_package( Threads REQUIRED )

find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core Widgets Gui PrintSupport )
find_package( Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS DataVisualization )

if ( NOT "${Qt${QT_VERSION_MAJOR}DataVisualization_FOUND}" )
  message( FATAL_ERROR "Qt5Datavisualization NOT FOUND" )
  return()
endif()

include( version )
set ( VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK} )

add_definitions( -D_LINUX -D_ACQIRIS -DVERSION="${VERSION}")


add_executable( ${PROJECT_NAME}
  main.cpp
  surfacegraph.h
  surfacegraph.cpp
  ${Resources}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adfs
  adplugin_manager
  adportable
  adprocessor
  ${Boost_LIBRARIES}
  ${QtDataVisualization_LIBRARIES}
  ${QtWidgets_LIBRARIES}
  Threads::Threads
  ${CMAKE_DL_LIBS}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )
