
project( socfpga )

add_definitions( -DSOCFPGA_LIBRARY )

#find_package( Qt5 REQUIRED Core )

add_library( socfpga SHARED
#  socfpga_global.hpp
  session.cpp
  session.hpp
  traceobserver.cpp
  traceobserver.hpp
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/contrib/linux
  ${Qt5Core_INCLUDE_DIRS}
  )

target_link_libraries( ${PROJECT_NAME}
  adacquire
  adplugin
  adportable
  adcontrols
  adfs
  adurl
  ${Boost_LIBRARIES}
  Threads::Threads
  Qt5::Core
  )

install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )
