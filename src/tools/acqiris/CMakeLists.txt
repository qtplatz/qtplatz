cmake_minimum_required( VERSION 3.0 )
project( acqiris )
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

set ( CMAKE_CXX_STANDARD 14 )
list( APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../../cmake/Modules" )

add_definitions( -D_LINUX -D_ACQIRIS )

find_package( Qt5 CONFIG REQUIRED Widgets Gui Svg Sql PrintSupport )
find_package( qwt )
find_package( Boost COMPONENTS system )
find_package( Threads REQUIRED )
if ( NOT QWT_LIBRARIES )
  message( FATAL_ERROR "No QWT library found" )
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/../../libs
  ${QWT_INCLUDE_DIRS}
  )

if ( WIN32 )
  set( ACQRSDIR "C:/Program Files (x86)/Agilent/Acqiris" )
  find_path( AqDrv4_INCLUDE_DIR NAME AcqirisImport.h PATHS "${ACQRSDIR}/include" )
  include_directories( ${AqDrv4_INCLUDE_DIR} "c:/Boost/include/boost-1_59" )
  # include( ${CMAKE_SOURCE_DIR}/../../../qtplatz.cmake )
endif()

add_executable( ${PROJECT_NAME}
  acqiris.cpp
  acqiris.hpp
  document.cpp
  document.hpp
  main.cpp
  mainwindow.cpp
  mainwindow.hpp
  task.cpp
  task.hpp
  waveform.cpp
  waveform.hpp
  waveformview.cpp
  waveformview.hpp
  xyseriesdata.cpp
  xyseriesdata.hpp
  ${Resources}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  AqDrv4
  ${QWT_LIBRARIES}
  ${Boost_LIBRARIES}
  Qt5::Widgets
  Qt5::Svg
  Threads::Threads
  dl
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )
