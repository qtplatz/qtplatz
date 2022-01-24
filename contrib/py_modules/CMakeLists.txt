
if ( NOT Python3_FOUND )
  message( FATAL_ERROR "##### Python3 requred #####")
endif()

if ( NOT Boost_PYTHON_FOUND )
  if ( WIN32 )
    message( STATUS  "##### boost_python was not found -- expecting auto link #####")
    add_definitions( -DBOOST_PYTHON_STATIC_LIB )
  else()
    message( FATAL_ERROR "##### Boost_PYTHON requred #####")
  endif()
endif()

if ( APPLE )
  set( PYTHON_MODULE_DIR ${CMAKE_BINARY_DIR}/bin/qtplatz.app/Library/Python/${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages/qtplatz )
else()
  set( PYTHON_MODULE_DIR ${CMAKE_BINARY_DIR}/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/qtplatz )
endif()

if ( WIN32 )
  #  set( PYTHON_DIST_PACKAGES "C:/Python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages" )
  set( PYTHON_DIST_PACKAGES "python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages" )
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
  message( STATUS "------------> cib/py_modules/py_adcontrols")
endif()

add_subdirectory( py_adcontrols )

if ( VERBOSE )
  message( STATUS "------------> contrib/py_modules/py_adprocessor")
endif()

add_subdirectory( py_adprocessor )

install( FILES "__init__.py"    DESTINATION "${PYTHON_DIST_PACKAGES}/qtplatz" COMPONENT py_modules )
install( FILES "jupyter-run.sh" DESTINATION bin COMPONENT py_modules )
install( DIRECTORY "../jupyter-notebook" DESTINATION share COMPONENT py_modules )
