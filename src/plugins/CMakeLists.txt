
function( print_message text )
  if ( VERBOSE )
    message( STATUS ${text} )
  endif()
endfunction()

include( ${CMAKE_SOURCE_DIR}/pluginconfig.cmake )

print_message( "<--------------- plugins -------------------------" )

if ( IDE_VERSION VERSION_GREATER_EQUAL "8.0.0" )
  if ( QTPLATZ_CONFIG_Example )
    add_subdirectory( example )
  endif()
  if ( QTPLATZ_CONFIG_HelloWorld )
    add_subdirectory( helloworld )
  endif()
endif()

##add_subdirectory( imageviewer )

if ( QTPLATZ_CONFIG_Servant )
  print_message( "----- Servant -----" )
  add_subdirectory( servant )
endif()

if ( QTPLATZ_CONFIG_Acquire )
  print_message( "----- Acquire -----" )
  add_subdirectory( acquire )
endif()

if ( QTPLATZ_CONFIG_Sequence )
  print_message( "----- Sequence -----" )
  add_subdirectory( sequence )
endif()

if ( QTPLATZ_CONFIG_Dataproc )
  print_message( "----- Dataproc -----" )
  add_subdirectory( dataproc )
endif()

if ( QTPLATZ_CONFIG_Chemistry AND rdkit_FOUND )
  print_message( "----- Chemistry -----" )
  add_subdirectory( chemistry )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND QTPLATZ_CONFIG_Cluster AND OpenCV_FOUND )
  print_message( "----- Cluster -----" )
  add_subdirectory( cluster )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND QTPLATZ_CONFIG_Peptide )
  print_message( "----- Peptide -----" )
  add_subdirectory( peptide )
endif()

if ( QTPLATZ_CONFIG_Quan )
  print_message( "----- Quan -----" )
  add_subdirectory( quan )
endif()

if ( QTPLATZ_CONFIG_Batch )
  print_message( "----- Batchproc -----" )
  add_subdirectory( batchproc )
endif()

if ( QTPLATZ_CONFIG_Query )
  print_message( "----- Query -----" )
  add_subdirectory( query )
endif()

if ( QTPLATZ_CONFIG_Video  AND NOT CMAKE_CROSSCOMPILING )
  print_message( "----- Video -----" )
  add_subdirectory( video )
endif()

if ( QTPLATZ_CONFIG_Lipidid AND rdkit_FOUND )
  print_message( "----- LipidId -----" )
  add_subdirectory( lipidid )
endif()

if ( QTPLATZ_CONFIG_SDFViewer )
  print_message( "----- SDFViewer -----" )
  add_subdirectory( sdfviewer )
endif()

if ( QTPLATZ_CONFIG_REST )
  add_subdirectory( rest )
endif()

print_message( "--------------- end plugins --------------------->" )
