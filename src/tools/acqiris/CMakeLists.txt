cmake_minimum_required( VERSION 3.5 )
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
set ( USING_CHARTS OFF )

set ( STANDALONE OFF )
set ( LOCAL_TCP_CLIENT OFF )

if ( STANDALONE )
  include( "standalone" )
endif()

add_definitions( -D_LINUX -D_ACQIRIS )

find_package( Qt5 CONFIG REQUIRED Widgets Gui Svg Sql PrintSupport )
find_package( qwt )
if ( NOT Boost_FOUND )
  find_package( Boost COMPONENTS filesystem system program_options serialization wserialization )
endif()
find_package( Threads REQUIRED )

add_definitions( -DPID_NAME="/var/run/${PROJECT_NAME}.pid" )

if ( USING_PROTOBUF )
  find_package( Protobuf REQUIRED )
  include_directories( ${PROTOBUF_INCLUDE_DIRS} )
  protobuf_generate_cpp( proto_sources proto_headers acqiris.proto )  
endif()

if ( WIN32 )
  set( ACQRSDIR "C:/Program Files (x86)/Agilent/Acqiris" )
  find_path( AqDrv4_INCLUDE_DIR NAME AcqirisImport.h PATHS "${ACQRSDIR}/include" )
  include_directories( ${AqDrv4_INCLUDE_DIR} "c:/Boost/include/boost-1_59" )
endif()

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  )

if ( NOT USING_CHARTS )
  set( qwt_dependent_sources
    waveformview.cpp
    waveformview.hpp
    xyseriesdata.cpp
    xyseriesdata.hpp
    )
else()
  add_definitions( -DUSING_CHARTS )
  find_package( Qt5 CONFIG REQUIRED Charts )
  set( chart_dependent_sources
    chartview.cpp
    chartview.hpp
    )
  set( chart_libraries Qt5::Charts )
endif()

add_executable( ${PROJECT_NAME}
  digitizer.cpp
  digitizer.hpp
  document.cpp
  document.hpp
  main.cpp
  mainwindow.cpp
  mainwindow.hpp
  task.cpp
  task.hpp
  tcp_connection.cpp
  tcp_connection.hpp  
  tcp_connection_manager.cpp
  tcp_connection_manager.hpp
  tcp_server.cpp
  tcp_server.hpp
  tcp_task.cpp
  tcp_task.hpp
  request_handler.cpp
  request_handler.hpp
  outputwidget.cpp
  outputwidget.hpp
  ${chart_dependent_sources} # disable
  ${qwt_dependent_sources}
  ${proto_sources}           # disable
  ${proto_headers}           # disable
  ${Resources}
  )

add_executable( ${PROJECT_NAME}d
  digitizer.cpp
  digitizer.hpp
  document.cpp
  document.hpp
  main.cpp
  task.cpp
  task.hpp
  tcp_connection.cpp
  tcp_connection.hpp  
  tcp_connection_manager.cpp
  tcp_connection_manager.hpp
  tcp_server.cpp
  tcp_server.hpp
  tcp_task.cpp
  tcp_task.hpp
  request_handler.cpp
  request_handler.hpp
  ${proto_sources}           # disable
  ${proto_headers}           # disable
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
  acqrswidgets
  AqDrv4
  ${QWT_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  ${Boost_LIBRARIES}
  ${chart_libraries}
  Qt5::Widgets
  Qt5::Svg
  Threads::Threads
  dl
  )

target_compile_definitions( ${PROJECT_NAME}d PRIVATE -DDAEMON=1 )

target_link_libraries( ${PROJECT_NAME}d LINK_PUBLIC
  adportable
  acqrscontrols
  AqDrv4
  ${PROTOBUF_LIBRARIES}
  ${Boost_LIBRARIES}
  Qt5::Core
  Threads::Threads
  dl
  )

install( TARGETS ${PROJECT_NAME} 
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )

install( TARGETS ${PROJECT_NAME}d
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )

