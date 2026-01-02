include( ${CMAKE_SOURCE_DIR}/pluginconfig.cmake )

if ( QTPLATZ_CONFIG_AccutofAcquire AND NOT WIN32 )
  add_subdirectory( accutofacquire )
endif()
