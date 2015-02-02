cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0028 NEW)

project( aggregation )

set(CMAKE_AUTOMOC ON)

add_definitions(-DAGGREGATION_LIBRARY)

find_package( Qt5 REQUIRED Core )
include_directories( ${Qt5Core_INCLUDE_DIRS} )

add_library(
  ${PROJECT_NAME} SHARED 
  aggregate.h
  aggregation_global.h
  aggregate.cpp
  )

target_link_libraries(
  ${PROJECT_NAME} LINK_PUBLIC
  Qt5::Core
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT apprications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
