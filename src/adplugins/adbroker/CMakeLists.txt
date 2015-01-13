# adbloer adplugin

project( adbroker )
#cmake_policy( SET CMP0020 NEW )
add_definitions(-DADBROKER_LIBRARY)

find_package( Boost 1.57 REQUIRED COMPONENTS regex date_time system filesystem wserialization serialization )
find_package( Qt5 REQUIRED Core )
find_package( ace+tao )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.adplugin"
  )

if ( ace+tao_FOUND )
  include_directories(
    ${ACE+TAO_INCLUDE_DIRS}
    ${Qt5Core_INCLUDE_DIRS}
    )
endif()

link_directories( ${ACE+TAO_LIBRARY_DIRS} )

add_library( adbroker MODULE
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

target_link_libraries( adbroker LINK_PUBLIC
  adinterface
  adfs
  adcontrols
  adportable
  portfolio
  adplugin
  adorbmgr
  adlog
  acewrapper
  optimized TAO_Utils debug TAO_Utilsd
  optimized TAO_PI debug TAO_PId
  optimized TAO_PortableServer debug TAO_PortableServerd
  optimized TAO_AnyTypeCode debug TAO_AnyTypeCoded
  optimized TAO debug TAOd
  optimized ACE debug ACEd
  ${Boost_LIBRARIES}
)



