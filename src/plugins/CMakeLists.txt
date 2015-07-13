
include( ${CMAKE_SOURCE_DIR}/pluginconfig.cmake )

add_subdirectory( coreplugin )
add_subdirectory( servant )

if ( QTPLATZ_CONFIG_Acquire )
  add_subdirectory( acquire )
endif()

if ( QTPLATZ_CONFIG_Sequence )
  add_subdirectory( sequence )
endif()

if ( QTPLATZ_CONFIG_Dataproc )
  add_subdirectory( dataproc )
endif()

if ( QTPLATZ_CONFIG_ChemSpider )
  add_subdirectory( chemspider )
endif()

if ( QTPLATZ_CONFIG_Chemistry AND rdkit_FOUND )
  add_subdirectory( chemistry )
endif()

if ( QTPLATZ_CONFIG_Peptide )
  add_subdirectory( peptide )
endif()

if ( QTPLATZ_CONFIG_Quan )
  add_subdirectory( quan )
endif()

if ( QTPLATZ_CONFIG_Batch )
  add_subdirectory( batchproc )
endif()

if ( QTPLATZ_CONFIG_Query )
  add_subdirectory( query )
endif()
