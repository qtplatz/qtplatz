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
set ( QTPLATZ_BUILD_DIR ${QTPLATZ_BUILD_DIR} CACHE STRING "destination directory" )
set ( USING_CHARTS OFF )

if ( NOT ${CMAKE_BUILD_TYPE} STREQUAL "" )
  string( TOLOWER ${CMAKE_BUILD_TYPE} build_type )
else()
  set( build_type "release" )
endif()

add_definitions( -D_LINUX -D_ACQIRIS )

set ( QTPLATZ_BUILD_DIR ${QTPLATZ_BUILD_DIR} CACHE STRING "destination directory" )

if ( NOT ${CMAKE_BUILD_TYPE} STREQUAL "" )
  string( TOLOWER ${CMAKE_BUILD_TYPE} build_type )
else()
  set( build_type "release" )
endif()

find_file( qtplatz_config NAME "qtplatz-config.cmake" PATHS
  "${QTPLATZ_BUILD_DIR}"
  "${CMAKE_BINARY_DIR}/../qtplatz.netbeans"
  "${CMAKE_BINARY_DIR}/../qtplatz.${build_type}"
  )
if ( NOT qtplatz_config )
  message( FATAL_ERROR "qtplatz-config.cmake can not be found" )
endif()

include( ${qtplatz_config} )

find_package( Qt5 CONFIG REQUIRED Widgets Gui Svg Sql PrintSupport )
find_package( qwt )
find_package( Boost COMPONENTS filesystem system program_options serialization wserialization )
find_package( Threads REQUIRED )

include_directories( ${QTPLATZ_INCLUDE_DIRS} ${QWT_INCLUDE_DIRS} )

if ( APPLE )
  include_directories( ${CMAKE_SOURCE_DIR}/../../../../infitof/acqiris/include )
endif()


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
  acqiris_method.cpp
  acqiris_method.hpp
  acqiris_protocol.cpp
  acqiris_protocol.hpp
  acqiriswidget.cpp
  acqiriswidget.hpp
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
  tcp_client.cpp
  tcp_client.hpp
  tcp_server.cpp
  tcp_server.hpp
  tcp_task.cpp
  tcp_task.hpp
  request_handler.cpp
  request_handler.hpp
  request_parser.cpp
  request_parser.hpp
  reply.cpp
  reply.hpp
  waveform.cpp
  waveform.hpp
  outputwidget.cpp
  outputwidget.hpp
  ${chart_dependent_sources} # disable
  ${qwt_dependent_sources}
  ${proto_sources}           # disable
  ${proto_headers}           # disable
  ${Resources}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
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

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )
