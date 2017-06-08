cmake_minimum_required (VERSION 2.8.9)
project( bnc565d CXX )

find_package( Boost 1.57 COMPONENTS date_time regex filesystem thread system program_options serialization REQUIRED )
set( Boost_USE_STATIC_LIBS ON )

list( APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../../cmake/Modules )

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )

  find_package( Threads )

  if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    find_package( Threads )
    configure_file( "${PROJECT_SOURCE_DIR}/httpd.sh.in" "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.sh" @ONLY )
    configure_file( "${PROJECT_SOURCE_DIR}/postinst.in" "${PROJECT_BINARY_DIR}/postinst" @ONLY )
    configure_file( "${PROJECT_SOURCE_DIR}/prerm.in" "${PROJECT_BINARY_DIR}/prerm" @ONLY )
  endif()
  
endif()

if ( MSVC )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601 -wd4267" )
  add_definitions(-DUNICODE -D_UNICODE)
endif()

add_definitions( -DPID_NAME="/var/run/${PROJECT_NAME}.pid" -DDOC_ROOT="${HTML_INSTALL_DIR}/html" )

set( http_server_SOURCES
  bnc565.cpp
  bnc565.hpp  
  connection.cpp
  connection_manager.cpp
  dgctl.cpp
  dgctl.hpp
  dgprotocols.cpp
  dgprotocols.hpp
  log.cpp
  log.hpp
  main.cpp
  mime_types.cpp
  reply.cpp
  request_handler.cpp
  request_parser.cpp
  server.cpp
  serialport.cpp
  serialport.hpp
  pugixml.cpp
  pugixml.hpp
  pugiconfig.hpp
  )

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/../linux/drivers
  ${QTPLATZ_SOURCE_DIR}/src/libs )

add_executable( ${PROJECT_NAME} ${http_server_SOURCES} ${Resources} )

set_property( TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14 )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  ${CMAKE_THREAD_LIBS_INIT}
  ${Boost_LIBRARIES}
  )
  
install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT httpd )
install( FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.sh DESTINATION /etc/init.d COMPONENT httpd )

