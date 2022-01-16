cmake_minimum_required( VERSION 3.5 )
project( sdfview )
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

set ( USING_CHARTS OFF )

if ( STANDALONE )
  include( "standalone.cmake" )
  find_package( AqDrv4 )
else()
  set( acqirisd_libraries  acqrscontrols )
endif()

include( version )
set ( VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK} )

add_definitions( -D_LINUX -D_ACQIRIS -DVERSION="${VERSION}")

find_package( Qt5 CONFIG REQUIRED Widgets Gui Svg Sql PrintSupport )
find_package( qwt )
find_package( Threads REQUIRED )

add_definitions( -DPID_NAME="/var/run/${PROJECT_NAME}.pid" )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/src/libs
  )

add_executable( ${PROJECT_NAME}
  main.cpp
  manhattanstyle.cpp
  manhattanstyle.hpp
  mainwindow.cpp
  mainwindow.hpp
  outputwidget.cpp
  outputwidget.hpp
  styleanimator.cpp
  styleanimator.hpp
  ${Resources}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
  utils
  ${Boost_LIBRARIES}
  Qt5::Widgets
  Qt5::Svg
  Threads::Threads
  ${CMAKE_DL_LIBS}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )

if ( AqDrv4_FOUND )

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
