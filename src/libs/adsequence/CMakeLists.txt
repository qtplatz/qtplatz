#-------------------------------------------------
project( adsequence )
#
#
cmake_policy( SET CMP0022 NEW )

add_definitions( -DADSEQUENCE_LIBRARY )
include_directories( ${QtCore_INCLUDE_DIRS} )

set( adsequence_SOURCES
  adsequence.cpp
  schema.cpp
  sequence.cpp
  streambuf.cpp
  )

set( adsequence_HEADERS
  adsequence.hpp
  adsequence_global.hpp
  schema.hpp
  sequence.hpp
  streambuf.hpp
  )

add_library( adsequence SHARED
  ${adsequence_SOURCES}
  ${adsequence_HEADERS}
  )

target_link_libraries( adsequence LINK_PUBLIC
  adportable
  adportable_serializer
  ${Boost_LIBRARIES}
  )

#set_target_properties( adsequence
#  PROPERTIES
#  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_BINARY_DIR}/lib/qtplatz )
runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
  LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
  ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
  )
