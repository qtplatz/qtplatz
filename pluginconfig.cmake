
if ( ${QT_VERSION_MAJOR} LESS 6 )
  set ( QTPLATZ_CONFIG_Acquire   ON )
  set ( QTPLATZ_CONFIG_Dataproc  ON )
  set ( QTPLATZ_CONFIG_Quan      ON )
  set ( QTPLATZ_CONFIG_Sequence  OFF )
  set ( QTPLATZ_CONFIG_Chemistry ${rdkit_FOUND} )
  set ( QTPLATZ_CONFIG_Peptide   ON )
  set ( QTPLATZ_CONFIG_Batch     OFF )
  set ( QTPLATZ_CONFIG_Query     ON )
  set ( QTPLATZ_CONFIG_Cluster   OFF )

  if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL "6.3.0" )
    set ( QTPLATZ_CONFIG_Lipidid OFF )
  else()
    set ( QTPLATZ_CONFIG_Lipidid ${COMPILER_SUPPORTS_CXX17} )
  endif()

endif()
