#
cmake_minimum_required (VERSION 2.8.9)

project ( libdgpio )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../modules
  ${Boost_INCLUDE_DIRS}
  )

add_library( dgpio pio.hpp pio.cpp )

target_link_libraries( dgpio ${Boost_LIBRARIES} )

install( TARGETS dgpio
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
