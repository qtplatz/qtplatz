
if ( NOT Python3_FOUND )
  message( FATAL_ERROR "##### Python3 requred #####")
endif()

if ( NOT Boost_PYTHON_FOUND )
  message( FATAL_ERROR "##### Boost_PYTHON requred #####")
endif()

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

if ( VERBOSE )
  message( STATUS "------------> contrib/py_modules PYTHON_DIST_PACKAGES: " ${PYTHON_DIST_PACKAGES})
  message( STATUS "------------> contrib/py_modules PYTHON_MODULE_DIR: " ${PYTHON_MODULE_DIR})
endif()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/__init__.py"
  "${PYTHON_MODULE_DIR}/__init__.py"
  )

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/jupyter-run.sh"
  "${PYTHON_MODULE_DIR}/jupyter-run.sh"
  COPYONLY
  )

add_subdirectory( world )

if ( VERBOSE )
  message( STATUS "------------> contrib/py_modules/adControls")
endif()
add_subdirectory( adControls )

if ( VERBOSE )
  message( STATUS "------------> contrib/py_modules/adProcessor")
endif()
add_subdirectory( adProcessor )

install( FILES  __init__.py DESTINATION "${PYTHON_DIST_PACKAGES}/qtplatz" COMPONENT modules )
install( FILES jupyter-run.sh DESTINATION bin COMPONENT modules )
install( DIRECTORY "../jupyter-notebook" DESTINATION share COMPONENT modules )
