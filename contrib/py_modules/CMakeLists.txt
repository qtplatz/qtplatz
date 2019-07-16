
find_package( Python3 COMPONENTS Interpreter Development )

if ( Python3_FOUND )

  if ( APPLE )
    set( PYTHON_MODULE_DIR ${CMAKE_BINARY_DIR}/bin/qtplatz.app/Library/Python/${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages/qtplatz )
  else()
    set( PYTHON_MODULE_DIR ${CMAKE_BINARY_DIR}/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/qtplatz )
  endif()

  if ( WIN32 )
    set( PYTHON_DIST_PACKAGES "C:/Python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages" )
  elseif( APPLE )
    set( PYTHON_DIST_PACKAGES "${CMAKE_INSTALL_PREFIX}/Library/Python/${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages" )
  else()
    set( PYTHON_DIST_PACKAGES "/usr/local/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/dist-packages" )
  endif()

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
    "${PYTHON_MODULE_DIR}/__init__.py"
    )

  add_subdirectory( world )
  add_subdirectory( adControls )
  add_subdirectory( adProcessor )

  install( FILES  __init__.py DESTINATION "${PYTHON_DIST_PACKAGES}/qtplatz" COMPONENT modules )

endif()
