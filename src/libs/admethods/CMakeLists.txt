cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0022 NEW )

project( admethods )

add_definitions(-DADMETHODS_LIBRARY)
if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions( -wd4251 -wd4244 )
endif()

if ( QT_FOUND )
  add_definitions( -DHAVE_Qt5=1 )  # this may inprove json read/write performance
  include_directories( ${QtCore_INCLUDES} )
  set ( qt_libraries Qt::Core )
endif()

add_library( ${PROJECT_NAME} SHARED
  admethods_global.hpp
  controlmethod/adtracemethod.cpp
  controlmethod/adtracemethod.hpp
  )

target_include_directories( ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
  adportable_serializer
  ${qt_libraries}
  ${Boost_LIBRARIES}
  )

#set_target_properties( adcontrols  PROPERTIES  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_BINARY_DIR}/lib/qtplatz )

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
  controlmethod/adtracemethod.hpp
  DESTINATION include/admethods/controlmethod COMPONENT headers
  )
