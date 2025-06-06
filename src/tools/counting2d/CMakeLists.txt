cmake_minimum_required( VERSION 3.5 )
project( counting2d )
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0028 NEW)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Widgets Gui Svg Sql PrintSupport )

if ( OpenCV_FOUND )
  add_definitions( "-DOPENCV=1" )
  include_directories( ${OpenCV_INCLUDE_DIRS} )
endif()

include_directories( ${QTPLATZ_INCLUDE_DIRS} ${QWT_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src )

add_executable( ${PROJECT_NAME}
  document.cpp
  document.hpp
  imageview.cpp
  imageview.hpp
  main.cpp
  mainwindow.cpp
  mainwindow.hpp
  spectrogramplot.cpp
  spectrogramplot.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
  adcontrols
  adfs
  adprocessor
  adplot
  adplugin_manager
  adwidgets
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${Resources}
  Qt::Widgets
  Qt::Svg
  Threads::Threads
  ${CMAKE_DL_LIBS}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )
