
project(cvplot)

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

if ( NOT OpenCV_FOUND )
  return()
endif()

add_definitions( "-DOPENCV=1" )

find_package( Qt5 CONFIG REQUIRED PrintSupport Core Widgets Gui )

include_directories(
  ${Qt5Widgets_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  )
		     
add_definitions(-DCVPLOT_LIBRARY)
if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

add_library( cvplot SHARED
  plot.cpp
  plot.hpp
  )

target_link_libraries( cvplot LINK_PUBLIC
  adportable
  ${OpenCV_LIBRARIES}
  Qt5::Widgets
  )

if( APPLE )
  install( TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION Contents/PlugIns COMPONENT libraries
    ARCHIVE DESTINATION Contents/PlugIns COMPONENT libraries )
else()
  install( TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
    ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
endif()
