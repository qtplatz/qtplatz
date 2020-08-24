cmake_minimum_required( VERSION 3.5 )
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0028 NEW)

project( adexport )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package( Qt5 CONFIG REQUIRED Core )

include_directories(
  ${CMAKE_SOURCE_DIR}/src
  ${Qt5Core_INCLUDE_DIRS}
  )

add_executable( ${PROJECT_NAME}
  main.cpp
  export.cpp
  export.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adfs
  adplugin_manager
  adprocessor
  acqrscontrols
  adportable
  ${Boost_LIBRARIES}
  Qt5::Core
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )
