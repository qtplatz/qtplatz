#
cmake_minimum_required (VERSION 2.8.9)

project ( libdgpio )

# --> lookup qtplatz build/config 
#find_file( qtplatz_config NAMES "qtplatz-config.cmake" HINTS ${QTPLATZ_BUILD_DIR} )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../modules
  ${Boost_INCLUDE_DIRS}
  )

add_library( dgpio pio.hpp pio.cpp )
add_executable( dgpio_test main.cpp )

target_link_libraries( dgpio ${Boost_LIBRARIES} )
target_link_libraries( dgpio_test dgpio )

install( TARGETS dgpio
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
