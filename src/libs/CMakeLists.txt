# adtoolkit

add_subdirectory ( adportable )
add_subdirectory ( adportable_serializer )
add_subdirectory ( adcontrols )
add_subdirectory ( admtcontrols )

add_subdirectory ( acewrapper )
add_subdirectory ( adchem )
add_subdirectory ( adfs )
add_subdirectory ( adio )
add_subdirectory ( adlog )
add_subdirectory ( admethods )
if ( QT_FOUND )
  add_subdirectory ( adplugin )  # <-- Qt5_Core dependency (QJsonXXX)
  add_subdirectory ( adacquire ) # <-- adplugin dependency
  #add_subdirectory ( adui )      # tools dependency
endif()
add_subdirectory ( adportfolio )
add_subdirectory ( adprot )
add_subdirectory ( adsequence )
add_subdirectory ( adurl )
add_subdirectory ( adutils )

add_subdirectory ( chromatogr )
add_subdirectory ( xmlparser )
add_subdirectory ( pug )

if ( OpenCV_FOUND OR ArrayFire_FOUND )
  add_subdirectory ( adcv )
endif()

if ( NOT CMAKE_CROSSCOMPILING AND QT_FOUND )

  add_subdirectory (adextension)
  add_subdirectory (adprocessor)
  add_subdirectory (adplugin_manager)
  add_subdirectory (adplot)
  add_subdirectory (qtwrapper)
  add_subdirectory (adwidgets)
  add_subdirectory (admtwidgets)

  if ( IDE_VERSION VERSION_LESS_EQUAL "3.2.81" )
    ######################## QtCreator3 ############################
    add_subdirectory (aggregation)
    add_subdirectory (extensionsystem)
    add_subdirectory (utils)
  endif()

  find_package( Qt${QT_VERSION_MAJOR} COMPONENTS XmlPatterns QUIET )
  find_package( Qt${QT_VERSION_MAJOR} COMPONENTS WebEngineWidgets QUIET )
  add_subdirectory (adpublisher)

endif()

install( FILES
  compiler/decl_export.h
  compiler/pragma_warning.hpp
  DESTINATION include/compiler COMPONENT headers
  )

install( FILES
  date/date.h
  DESTINATION include/date COMPONENT headers
  )

if ( UNIX AND CMAKE_CROSSCOMPILING )
  install ( FILES qtplatz.conf DESTINATION /etc/ld.so.conf.d COMPONENT runtime_libraries )
endif()

if ( CMAKE_SYSTEM_NAME STREQUAL "Linux" )
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/qtplatz.conf.in"
    "${CMAKE_BINARY_DIR}/etc/qtplatz.conf"
    @ONLY
  )
  install ( FILES "${CMAKE_BINARY_DIR}/etc/qtplatz.conf" DESTINATION "/etc/ld.so.conf.d" COMPONENT runtime_libraries )
endif()
