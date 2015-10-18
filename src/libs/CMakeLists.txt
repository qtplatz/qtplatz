# adtoolkit 

add_subdirectory (acewrapper)
add_subdirectory (adportable)
add_subdirectory (adcontrols)
add_subdirectory (adsequence)
add_subdirectory (adfs)
add_subdirectory (adicontroller)
add_subdirectory (adlog)
add_subdirectory (adportfolio)
add_subdirectory (adprot)
add_subdirectory (adutils)
add_subdirectory (chromatogr)
add_subdirectory (xmlparser)

if ( QTPLATZ_SUPPORT_CORBA )
  add_subdirectory (adorbmgr)
  add_subdirectory (adinterface)
endif()

add_subdirectory (adplugin)

if ( rdkit_FOUND )
  add_subdirectory (adchem)
endif()

if ( Qt5_FOUND )
  
  add_subdirectory (qtwrapper)
  add_subdirectory (adwidgets)
  add_subdirectory (adplugin_manager)
  
  if ( NOT QTPLATZ_CORELIB_ONLY )  
    add_subdirectory (adplot)  # this raize gcc internal compiler error at qwt header
    add_subdirectory (adextension)
    add_subdirectory (aggregation)    
    add_subdirectory (extensionsystem)
    add_subdirectory (utils)
    
    find_package( Qt5 COMPONENTS XmlPatterns QUIET )
    
    if ( Qt5XmlPatterns_FOUND )
      add_subdirectory (adpublisher)
    endif()

  endif()

endif()

message( STATUS "------------ end libs ----------------" )
