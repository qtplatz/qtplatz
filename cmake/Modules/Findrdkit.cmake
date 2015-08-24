# find RDKit

if ( rdkit_FOUND )
  return()
endif()

# when run 'make install' or 'INSTALL.vcxproj', .libs are copied into RDKit's ${CMAKE_SOURCE_DIR}/rdkit/lib

find_package( rdkit CONFIG HINTS
  ${CMAKE_SOURCE_DIR}/../rdkit/lib
  $ENV{RDBASE}
  /usr/local/lib
  )

if ( rdkit_FOUND )
  
  set( RDKit_LIBRARY_DIRS ${_dir} )  # <- set by rdkit-config.cmake
  # RDKit_INCLUDE_DIRS has been set by rdkit-config.cmake
  
else()

  if ( WIN32 )
    message( STATUS "Findrdkit: rdkit-config.cmake could not be found, so abundan search." )
    return()
  endif()
  
  find_path( _include_dir GraphMol/RDKitBase.h HINTS
    $ENV{RDBASE}/Code
    ${CMAKE_SOURCE_DIR}/../rdkit/Code
    /usr/local/include/rdkit
    /usr/include/rdkit
    )

  if ( _include_dir )
    message( STATUS "## Findrdkit: found by include_dir search: "  ${_include_dir} )
  else()
    return()
  endif()

  find_library( _fileparsers_lib NAMES FileParsers HINTS
    ${_include_dir}/../lib
    /usr/local/lib
    /usr/lib )
  
  if ( _fileparsers_lib )
    get_filename_component ( _libdir ${_fileparsers_lib} PATH )
  else()
    return()
  endif()

  message( "##### _fileparsers_lib: " ${_fileparsers_lib} )

  set ( rdkit_FOUND TRUE )
  set ( RDKit_INCLUDE_DIRS ${_include_dir} )
  set ( RDKit_LIBRARY_DIRS ${_libdir} )

  find_library(SMILESPARSE_LIB    NAMES SmilesParse   HINTS ${_libdir})
  find_library(DEPICTOR_LIB       NAMES Depictor      HINTS ${_libdir})
  find_library(DESCRIPTORS_LIB    NAMES Descriptors   HINTS ${_libdir})  
  find_library(GRAPHMOL_LIB       NAMES GraphMol      HINTS ${_libdir})
  find_library(RDGEOMETRYLIB_LIB  NAMES RDGeometryLib HINTS ${_libdir})
  find_library(RDGENERAL_LIB      NAMES RDGeneral     HINTS ${_libdir})
  find_library(SUBSTRUCTMATCH_LIB NAMES SubstructMatch HINTS ${_libdir})

  add_library( FileParsers SHARED IMPORTED )
  add_library( GraphMol    SHARED IMPORTED )
  add_library( SmilesParse SHARED IMPORTED )
  add_library( RDGeneral   SHARED IMPORTED )
  add_library( RDGeometryLib SHARED IMPORTED )
  add_library( SubstructMatch SHARED IMPORTED )
  add_library( Depictor    SHARED IMPORTED )
  add_library( Descriptors SHARED IMPORTED )

  set_target_properties( FileParsers PROPERTIES IMPORTED_LOCATION ${_fileparsers_lib} )
  set_target_properties( GraphMol    PROPERTIES IMPORTED_LOCATION ${GRAPHMOL_LIB} )
  set_target_properties( SmilesParse PROPERTIES IMPORTED_LOCATION ${SMILESPARSE_LIB} )
  set_target_properties( RDGeneral   PROPERTIES IMPORTED_LOCATION ${RDGENERAL_LIB} )
  set_target_properties( RDGeometryLib  PROPERTIES IMPORTED_LOCATION ${RDGEOMETRYLIB_LIB} )
  set_target_properties( SubstructMatch PROPERTIES IMPORTED_LOCATION ${SUBSTRUCTMATCH_LIB} )
  set_target_properties( Depictor    PROPERTIES IMPORTED_LOCATION ${DEPICTOR_LIB} )
  set_target_properties( Descriptors PROPERTIES IMPORTED_LOCATION ${DESCRIPTORS_LIB} )
  
endif()

find_file( version_cmake NAMES rdkit-config-version.cmake PATHS ${RDKit_LIBRARY_DIRS} NO_DEFAULT_PATH )
if ( version_cmake )
  include( ${version_cmake} )
  set( RDKit_PACKAGE_VERSION ${PACKAGE_VERSION} )
endif()

set (RDKit_LIBRARIES
  FileParsers
  SmilesParse
  Depictor
  Descriptors
  GraphMol
  RDGeometryLib
  RDGeneral
  SubstructMatch
  )


