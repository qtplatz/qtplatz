cmake_minimum_required( VERSION 3.5 )
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0028 NEW)

project( adimport )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${CMAKE_SOURCE_DIR}/src
  )

add_executable( ${PROJECT_NAME}
  main.cpp
  document.cpp
  document.hpp
  resultwriter.cpp
  resultwriter.hpp
  ${CMAKE_SOURCE_DIR}/src/adplugins/adtextfile/txtspectrum.cpp
  ${CMAKE_SOURCE_DIR}/src/adplugins/adtextfile/txtspectrum.hpp
  ${CMAKE_SOURCE_DIR}/src/adplugins/adtextfile/dialog.cpp
  ${CMAKE_SOURCE_DIR}/src/adplugins/adtextfile/dialog.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adfs
  adplugin_manager
  adprocessor
  acqrscontrols
  adportable
  ${Boost_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )
