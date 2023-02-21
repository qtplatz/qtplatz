
include( ${CMAKE_SOURCE_DIR}/pluginconfig.cmake )

if ( ${QT_VERSION_MAJOR} GREATER_EQUAL 6 )
  if ( QTPLATZ_CONFIG_Example )
    add_subdirectory( example )
  endif()
  if ( QTPLATZ_CONFIG_HelloWorld )
    add_subdirectory( helloworld )
  endif()
endif()

if ( ${QT_VERSION_MAJOR} EQUAL 5 )
  message( STATUS "================== Qt5 core plugins ======================" )
  add_subdirectory( coreplugin )
endif()

if ( QTPLATZ_CONFIG_Servant )
  add_subdirectory( servant )
endif()

if ( QTPLATZ_CONFIG_Acquire )
  add_subdirectory( acquire )
endif()

if ( QTPLATZ_CONFIG_Sequence )
  add_subdirectory( sequence )
endif()

if ( QTPLATZ_CONFIG_Dataproc )
  add_subdirectory( dataproc )
endif()

if ( QTPLATZ_CONFIG_Chemistry AND rdkit_FOUND )
  add_subdirectory( chemistry )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND QTPLATZ_CONFIG_Cluster AND OpenCV_FOUND )
  add_subdirectory( cluster )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND QTPLATZ_CONFIG_Peptide )
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

if ( NOT CMAKE_CROSSCOMPILING AND OpenCV_FOUND )
  add_subdirectory( video )
endif()

if ( QTPLATZ_CONFIG_Lipidid )
  add_subdirectory( lipidid )
endif()

# add_subdirectory( welcome )
