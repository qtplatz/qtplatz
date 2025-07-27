
project( adio )

add_definitions(-DADIO_LIBRARY)

add_library(
  ${PROJECT_NAME} SHARED
  dgprotocols.cpp
  dgprotocols.hpp
  dgprotocol.hpp
  ioconfig.cpp
  ioconfig.hpp
  ioeventsequence.cpp
  ioeventsequence.hpp
  iosequence.cpp
  iosequence.hpp
  )

target_include_directories( adio PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:include/adio>"
)

target_link_libraries( ${PROJECT_NAME}
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

install( FILES
  dgprotocol.hpp
  dgprotocols.hpp
  ioconfig.hpp
  ioeventsequence.hpp
  iosequence.hpp
  DESTINATION include/adio COMPONENT headers
  )
