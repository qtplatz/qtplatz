#
cmake_minimum_required(VERSION 2.8.11)
project( mac_installer )

include( qtplatz-version )

set( QTPLATZ_VERSION ${git_describe} )
set( RDKIT_LIBRARY_DIR ${RDKit_LIBRARY_DIRS} )
set( RDKIT_LIBRARIES )
foreach( _rdlib ${RDKit_LIBRARIES} )
  set( RDKIT_LIBRARIES "${RDKIT_LIBRARIES} ${_rdlib}" )
endforeach( _rdlib )

set( QTPLATZ_SOURCE_DIR "${CMAKE_SOURCE_DIR}" )
set( QTPLATZ_BINARY_DIR "${CMAKE_BINARY_DIR}" )

configure_file(
  "${PROJECT_SOURCE_DIR}/makefile.in"
  "${CMAKE_BINARY_DIR}/package/makefile"
  @ONLY
  )

