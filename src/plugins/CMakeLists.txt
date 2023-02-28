
function( print_message text )
  if ( VERBOSE )
    message( STATUS ${text} )
  endif()
endfunction()

include( ${CMAKE_SOURCE_DIR}/pluginconfig.cmake )

if ( ${QT_VERSION_MAJOR} GREATER_EQUAL 6 )
  if ( QTPLATZ_CONFIG_Example )
    add_subdirectory( example )
  endif()
  if ( QTPLATZ_CONFIG_HelloWorld )
    add_subdirectory( helloworld )
  endif()
endif()

## add_subdirectory( imageviewer )
################### QtCreator4 #########################
if ( ${QT_VERSION_MAJOR} EQUAL 5 )
  add_subdirectory( coreplugin )
endif()
################### END QtCreator4 #####################

if ( QTPLATZ_CONFIG_Servant )
  print_message( "----- Servant plugin -----" )
  add_subdirectory( servant )
endif()

if ( QTPLATZ_CONFIG_Acquire )
  print_message( "----- Acquire plugin -----" )
  add_subdirectory( acquire )
endif()

if ( QTPLATZ_CONFIG_Sequence )
  print_message( "----- Sequence plugin -----" )
  add_subdirectory( sequence )
endif()

if ( QTPLATZ_CONFIG_Dataproc )
  print_message( "----- Dataproc plugin -----" )
  add_subdirectory( dataproc )
endif()

if ( QTPLATZ_CONFIG_Chemistry AND rdkit_FOUND )
  print_message( "----- Chemistry plugin -----" )
  add_subdirectory( chemistry )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND QTPLATZ_CONFIG_Cluster AND OpenCV_FOUND )
  print_message( "----- Cluster plugin -----" )
  add_subdirectory( cluster )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND QTPLATZ_CONFIG_Peptide )
  print_message( "----- Peptide plugin -----" )
  add_subdirectory( peptide )
endif()

if ( QTPLATZ_CONFIG_Quan )
  print_message( "----- Quan plugin -----" )
  add_subdirectory( quan )
endif()

if ( QTPLATZ_CONFIG_Batch )
  print_message( "----- Batchproc plugin -----" )
  add_subdirectory( batchproc )
endif()

if ( QTPLATZ_CONFIG_Query )
  print_message( "----- Query plugin -----" )
  add_subdirectory( query )
endif()

if ( QTPLATZ_CONFIG_Video  AND NOT CMAKE_CROSSCOMPILING )
  print_message( "----- Video plugin -----" )
  add_subdirectory( video )
endif()

if ( QTPLATZ_CONFIG_Lipidid )
  print_message( "----- LipidId plugin -----" )
  add_subdirectory( lipidid )
endif()
