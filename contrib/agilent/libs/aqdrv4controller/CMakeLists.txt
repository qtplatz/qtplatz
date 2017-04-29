# ---- malpix/src/libs/mpxcontroller ----

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

project( aqdrv4controller )

add_definitions(-DAQDRV4CONTROLLER_LIBRARY -D_LINUX -D_ACQIRIS )

include_directories(
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${QWT_INCLUDE_DIRS}
  ${AqDrv4_INCLUDE_DIR}
  )

set( _sources
  aqdrv4controller.cpp
  waveformobserver.cpp
  waveformobserver.hpp
  session.cpp
  session.hpp
  task.cpp
  task.hpp
  )

add_library( ${PROJECT_NAME} SHARED ${_sources} )

target_include_directories( ${PROJECT_NAME} PUBLIC
  ${QTPLATZ_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SORUCE_DIR}/src/libs )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adicontroller
  adplugin
  acqrscontrols
  adportable
  adutils
  adfs
  ${Boost_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
