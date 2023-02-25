
project( adportable_serializer )

add_definitions(-DADPORTABLE_LIBRARY)

if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions( -wd4251 -wd4244 )
endif()

add_library( adportable_serializer
  portable_binary_oarchive.cpp
  portable_binary_oarchive.hpp
  portable_binary_iarchive.cpp
  portable_binary_iarchive.hpp
  )

target_include_directories( ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib COMPONENT libraries ) # .a
