
project( socfpga )

add_definitions( -DSOCFPGA_LIBRARY )

#find_package( Qt${QT_VERSION_MAJOR} REQUIRED Core )

add_library( socfpga SHARED
#  socfpga_global.hpp
  session.cpp
  session.hpp
  traceobserver.cpp
  traceobserver.hpp
  data_accessor.cpp
  data_accessor.hpp
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/contrib/linux
  ${QtCore_INCLUDE_DIRS}
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
  Qt::Core
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
