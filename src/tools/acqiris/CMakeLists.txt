cmake_minimum_required( VERSION 3.5 )
project( acqiris )
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0028 NEW)

set ( STANDALONE OFF CACHE BOOL "STANDALONE" )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set ( CMAKE_CXX_STANDARD 14 )
set ( USING_CHARTS OFF )

if ( STANDALONE )
  include( "standalone.cmake" )
else()
  set( acqirisd_libraries  acqrscontrols )
endif()

include( version )
set ( VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK} )

add_definitions( -D_LINUX -D_ACQIRIS -DVERSION="${VERSION}")

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

if ( APPLE )
  add_definitions( -DHAVE_AqDrv4=0 )
  if ( STANDALONE )
    include_directories( "${CMAKE_SOURCE_DIR}/../../../contrib/agilent/include" )
  else()
    include_directories( "${CMAKE_SOURCE_DIR}/contrib/agilent/include" )
  endif()
  set( aqdrv4_sources )  
else() #Linux
  if ( AqDrv4_LIBRARY )
    add_definitions( -DHAVE_AqDrv4=1 )
    set( AqDrv4_LIBRARIES ${AqDrv4_LIBRARY} )
    set( aqdrv4_sources
      digitizer.cpp
      digitizer.hpp
      task.cpp
      task.hpp
      )
  else()
    add_definitions( -DHAVE_AqDrv4=0 )
    include_directories( ${AqDrv4_INCLUDE_DIR} )
  endif()
endif()

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  )

set( qwt_dependent_sources
  waveformview.cpp
  waveformview.hpp
  xyseriesdata.cpp
  xyseriesdata.hpp
  )

add_executable( ${PROJECT_NAME}
  app.cpp
  app.hpp
  document.cpp
  document.hpp
  main.cpp
  mainwindow.cpp
  mainwindow.hpp
  mediator.cpp
  mediator.hpp
  ${aqdrv4_sources}
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
  ${qwt_dependent_sources}
  ${proto_sources}           # disable
  ${proto_headers}           # disable
  ${Resources}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
  acqrscontrols
  acqrswidgets
  ${AqDrv4_LIBRARIES}
  ${QWT_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  ${Boost_LIBRARIES}
  Qt5::Widgets
  Qt5::Svg
  Threads::Threads
  dl
  )

install( TARGETS ${PROJECT_NAME} 
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )

if ( AqDrv4_LIBRARY )

  add_executable( ${PROJECT_NAME}d
    daemon.cpp
    daemon.hpp
    digitizer.cpp
    digitizer.hpp
    log.cpp
    log.hpp
    main.cpp
    mediator.cpp
    mediator.hpp
    task.cpp
    task.hpp
    tcp_connection.cpp
    tcp_connection.hpp  
    tcp_connection_manager.cpp
    tcp_connection_manager.hpp
    tcp_server.cpp
    tcp_server.hpp
    request_handler.cpp
    request_handler.hpp
    ${standalone_additional_sources}
    ${proto_sources}           # disable
    ${proto_headers}           # disable
    )
  target_compile_definitions( ${PROJECT_NAME}d PRIVATE -DACQIRIS_DAEMON=1 )

  target_link_libraries( ${PROJECT_NAME}d LINK_PUBLIC
    adportable
    AqDrv4
    ${acqirisd_libraries}
    ${PROTOBUF_LIBRARIES}
    ${Boost_LIBRARIES}
    Qt5::Core
    Threads::Threads
    dl
    )

  install( TARGETS ${PROJECT_NAME}d
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT applications
    LIBRARY DESTINATION lib/qtplatz COMPONENT libraries 
    ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
    )
  
endif()

