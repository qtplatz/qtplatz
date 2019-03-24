cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0022 NEW )

project( admethods )

add_definitions(-DADCONTROLS_LIBRARY)
if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions( -wd4251 -wd4244 )
endif()

if ( Qt5_FOUND )
  add_definitions( -DHAVE_Qt5=1 )  # this may inprove json read/write performance
  include_directories( ${Qt5Core_INCLUDES} )
  set ( qt5_libraries Qt5::Core )
endif()

add_library( ${PROJECT_NAME} SHARED
  admethods_global.hpp
  controlmethod/adtracemethod.cpp
  controlmethod/adtracemethod.hpp
  )

target_include_directories( ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
  ${qt5_libraries}
  ${Boost_LIBRARIES}
  )

#set_target_properties( adcontrols  PROPERTIES  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_BINARY_DIR}/lib/qtplatz )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries
  ARCHIVE DESTINATION lib COMPONENT libraries )

install( FILES
  controlmethod/adtracesmmethod.hpp
  DESTINATION include/admethods/controlmethod COMPONENT headers
  )
