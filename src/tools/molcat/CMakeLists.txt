cmake_minimum_required( VERSION 3.5 )
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0028 NEW)

project( molcat )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#  find_package( BZip2 REQUIRED )

include_directories(
  ${CMAKE_SOURCE_DIR}/src
  ${RDKit_INCLUDE_DIRS}
  )

add_executable( ${PROJECT_NAME}
  main.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
  adchem
  ${Boost_LIBRARIES}
  ${RDKit_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )
