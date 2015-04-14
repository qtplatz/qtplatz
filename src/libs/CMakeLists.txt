# adtoolkit 

add_subdirectory (acewrapper)
add_subdirectory (adportable)
add_subdirectory (adcontrols)
add_subdirectory (adsequence)
add_subdirectory (adfs)
add_subdirectory (adlog)
add_subdirectory (adprot)
add_subdirectory (adutils)
add_subdirectory (chromatogr)
add_subdirectory (xmlparser)
add_subdirectory (portfolio)

add_subdirectory (adorbmgr)
add_subdirectory (adinterface)
add_subdirectory (adplugin)

if ( Qt5_FOUND )
  
  add_subdirectory (qtwrapper)
  add_subdirectory (adwidgets)
  add_subdirectory (adplot)

  if ( NOT QTPLATZ_CORELIB_ONLY )

    add_subdirectory (adextension)
    
    find_package( Qt5 COMPONENTS Script QUIET )
    
    if ( Qt5Script_FOUND )
      add_subdirectory (extensionsystem)
      add_subdirectory (utils)
    endif()
    
    add_subdirectory (aggregation)
    
    find_package( Qt5 COMPONENTS XmlPatterns QUIET )
    
    if ( Qt5XmlPatterns_FOUND )
      add_subdirectory (adpublisher)
    endif()

  endif()

endif()

if ( rdkit_DIR )
  add_subdirectory (adchem)
endif()
