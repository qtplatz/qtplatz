#
cmake_minimum_required (VERSION 2.8.9)

project ( diag-u5303a )

if ( WIN32 )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS )
  set( IVIROOTDIR "C:/Program Files/IVI Foundation/IVI" )
  set( VXIPNPPATH "C:/Program Files/IVI Foundation/VISA" )
endif()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${QTPLATZ_SOURCE_DIR}/contrib/agilent/libs
  ${QTPLATZ_SOURCE_DIR}/contrib/linux
  ${QTPLATZ_SOURCE_DIR}/src/libs
  ${AgMD2_INCLUDE_DIR}
  )

set( SOURCES test_u5303a.cpp )

add_executable( ${PROJECT_NAME} main.cpp )

link_directories( ${QTPLATZ_SOURCE_DIR}/../build-x86_64/qtplatz.debug/lib/qtplatz )

target_link_libraries( ${PROJECT_NAME}
  u5303a
  adcontrols
  acqrscontrols
  dgpio
  ${AgMD2_LIBRARIES}
)

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
