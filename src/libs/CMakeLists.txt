# adtoolkit 

add_subdirectory (adportable)
add_subdirectory (adcontrols)
add_subdirectory (acewrapper)
add_subdirectory (adfs)
add_subdirectory (adutils)
add_subdirectory (adprot)
add_subdirectory (xmlparser)
add_subdirectory (adlog)
add_subdirectory (portfolio)

if ( ACE_ROOT )
  add_subdirectory (adorbmgr)
  add_subdirectory (adinterface)
endif()

if ( NOT RTC_ARCH_ARM )

  add_subdirectory (adplugin)
  add_subdirectory (qtwrapper)
  add_subdirectory (adplot)
  add_subdirectory (adwidgets)
  add_subdirectory (utils)

  if ( RDKIT_CONFIG_DIR )
    add_subdirectory (adchem)
  endif()

endif()
