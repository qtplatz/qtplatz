
include( ${CMAKE_SOURCE_DIR}/pluginconfig.cmake )

add_subdirectory( coreplugin )
add_subdirectory( servant )

if ( QTPLATZ_CONFIG_Acquire )
  message( STATUS "------------ src/plugins/acquire --------------------" )
  add_subdirectory( acquire )
endif()

if ( QTPLATZ_CONFIG_Sequence )
    message( STATUS "------------ src/plugins/sequence --------------------" )
  add_subdirectory( sequence )
endif()

if ( QTPLATZ_CONFIG_Dataproc )
  message( STATUS "------------ src/plugins/dataproc --------------------" )
  add_subdirectory( dataproc )
endif()

if ( QTPLATZ_CONFIG_Chemistry AND rdkit_FOUND )
  message( STATUS "------------ src/plugins/chemistry --------------------" )
  add_subdirectory( chemistry )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND QTPLATZ_CONFIG_Cluster AND OpenCV_FOUND )
  add_subdirectory( cluster )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND QTPLATZ_CONFIG_Peptide )
  message( STATUS "------------ src/plugins/peptide --------------------" )
  add_subdirectory( peptide )
endif()

if ( QTPLATZ_CONFIG_Quan )
  message( STATUS "------------ src/plugins/quan --------------------" )
  add_subdirectory( quan )
endif()

if ( QTPLATZ_CONFIG_Batch )
  message( STATUS "------------ src/plugins/batch --------------------" )
  add_subdirectory( batchproc )
endif()

if ( QTPLATZ_CONFIG_Query )
  message( STATUS "------------ src/plugins/query --------------------" )
  add_subdirectory( query )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND OpenCV_FOUND )
  message( STATUS "------------ src/plugins/video --------------------" )
  add_subdirectory( video )
endif()
