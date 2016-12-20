set ( QTPLATZ_CONFIG_Acquire ${QTPLATZ_SUPPORT_CORBA} )
set ( QTPLATZ_CONFIG_Dataproc ON )
if ( WIN32 )
  set ( QTPLATZ_CONFIG_Quan ON )
else()
  set ( QTPLATZ_CONFIG_Quan ON )  
endif()
set ( QTPLATZ_CONFIG_Sequence OFF )
set ( QTPLATZ_CONFIG_Chemistry ${rdkit_FOUND} )
  
set ( QTPLATZ_CONFIG_Peptide ON )
set ( QTPLATZ_CONFIG_Batch OFF )
set ( QTPLATZ_CONFIG_Query ON )

