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
  adacquire
  adplugin
  acqrscontrols
  adportable
  adutils
  adfs
  ${Boost_LIBRARIES}
  )

runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
  LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
  ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
  )
