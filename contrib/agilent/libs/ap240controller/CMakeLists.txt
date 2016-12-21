# ---- malpix/src/libs/mpxcontroller ----

cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

project( ap240controller )

if ( NOT Boost_FOUND )
  #find_package(Boost 1.56 COMPONENTS date_time system filesystem bzip2 iostreams wserialization serialization)
endif()

add_definitions(-DAP240CONTROLLER_LIBRARY)

include_directories(
  ${CMAKE_SOURCE_DIR}/src/libs
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${QWT_INCLUDE_DIRS}
  )

set( _sources
  ap240controller.cpp
  waveformobserver.hpp
  waveformobserver.cpp
  session.hpp
  session.cpp
  )

add_library( ${PROJECT_NAME} SHARED ${_sources} )

target_include_directories( ${PROJECT_NAME} PUBLIC
  ${QTPLATZ_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SORUCE_DIR}/src/libs )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adicontroller
  adplugin
  ap240
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
