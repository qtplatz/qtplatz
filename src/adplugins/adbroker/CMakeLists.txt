project( adbroker )

add_definitions(-DADBROKER_LIBRARY)

if ( MSVC )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
endif()

find_package( ace+tao )
find_package( Boost 1.57 REQUIRED COMPONENTS regex date_time system filesystem wserialization serialization )
find_package( Qt5 REQUIRED Core )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.adplugin"
  )

include_directories(
  ${CMAKE_BINARY_DIR}/src/libs/adinterface
  ${CMAKE_BINARY_DIR}/src/libs
  ${ACE+TAO_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  )

link_directories( ${ACE+TAO_LIBRARY_DIRS} )
message( "TAO_LIBRARY_DIRS" ${TAO_LIBRARY_DIRS} )

add_library( ${PROJECT_NAME} MODULE
  adbroker.cpp
  brokerconfig.cpp
  brokermanager.cpp
  brokersession.cpp
  brokertoken.cpp
  logger_i.cpp
  manager_i.cpp
  session_i.cpp
  task.cpp
  objectdiscovery.cpp
  orbbroker.cpp
  adbroker.hpp
  adbroker_global.h
  brokermanager.hpp
  brokerconfig.hpp
  brokermanager.hpp
  brokersession.hpp
  brokertoken.hpp
  logger_i.hpp
  manager_i.hpp
  session_i.hpp
  task.hpp
  objectdiscovery.hpp
  orbbroker.hpp
  )

if ( CMAKE_SYSTEM_NAME EQUAL Linux )
  set( linux_LIBS dl rt )
endif()

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adorbmgr
  acewrapper
  adinterface
  adfs
  adcontrols
  adportable
  adplugin
  xmlparser
  adlog
  ${ACE+TAO_LIBRARIES}  
  ${Boost_LIBRARIES}
  ${linux_LIBS}
)
