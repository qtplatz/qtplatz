# adtoolkit 

add_subdirectory (acewrapper)
add_subdirectory (adcontrols)
add_subdirectory (adfs)
add_subdirectory (adicontroller)
add_subdirectory (adlog)
add_subdirectory (adportable)
add_subdirectory (adprocessor)
add_subdirectory (adsequence)
add_subdirectory (adportfolio)
add_subdirectory (adprot)
add_subdirectory (adurl)
add_subdirectory (adutils)
add_subdirectory (chromatogr)
add_subdirectory (xmlparser)

add_subdirectory (adplugin)

if ( rdkit_FOUND )
  add_subdirectory ( adchem )
endif()

if ( NOT QTPLATZ_CORELIB_ONLY AND Qt5_FOUND )

  add_subdirectory (adextension)
  add_subdirectory (qtwrapper)
  add_subdirectory (adwidgets)
  add_subdirectory (adplugin_manager)
  add_subdirectory (adplot)
  add_subdirectory (aggregation)    
  add_subdirectory (extensionsystem)
  add_subdirectory (utils)
  
  find_package( Qt5 COMPONENTS XmlPatterns QUIET )
  find_package( Qt5 COMPONENTS WebEngineWidgets QUIET )
  
  if ( Qt5XmlPatterns_FOUND AND Qt5WebEngineWidgets_FOUND )
    add_subdirectory (adpublisher)
  endif()

endif()

