# find RDKit

if ( rdkit_FOUND )
  return()
endif()

set( rdkit_FOUND FALSE )

find_path( _include_dir  GraphMol/RDKitBase.h HINTS
  $ENV{RDBASE}/Code
  ${CMAKE_SOURCE_DIR}/../rdkit/Code
  /usr/local/include/rdkit
  /usr/include/rdkit
  )

if ( _include_dir )
  get_filename_component ( rdbase ${_include_dir} PATH )
  if ( ${rdbase} MATCHES "/usr/include" )
    set( rdbase "/usr" )
  endif()
  if ( ${rdbase} MATCHES "/usr/local/include" )
    set( rdbase "/usr/local" )
  endif()
else()
  message( STATUS "Findrdkit: rdkit not found" )
  return()
endif()

find_library( _fileparsers_lib NAMES FileParsers HINTS
  ${_include_dir}/../lib
  /usr/local/lib
  /usr/lib )

if ( _fileparsers_lib )
  get_filename_component ( _libdir ${_fileparsers_lib} PATH )
endif()

if ( _include_dir AND _libdir )
  
  set ( rdkit_FOUND TRUE )
  set ( RDKit_INCLUDE_DIRS ${_include_dir} )
  set ( RDKit_LIBRARY_DIRS ${_libdir} )

  find_library(SMILESPARSE_LIB   NAMES SmilesParse   HINTS ${_libdir})
  find_library(DEPICTOR_LIB      NAMES Depictor      HINTS ${_libdir})
  find_library(GRAPHMOL_LIB      NAMES GraphMol      HINTS ${_libdir})
  find_library(RDGEOMETRYLIB_LIB NAMES RDGeometryLib HINTS ${_libdir})
  find_library(RDGENERAL_LIB     NAMES RDGeneral     HINTS ${_libdir})
  find_library(SUBSTRUCTMATCH_LIB NAMES SubstructMatch HINTS ${_libdir})

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

  find_file( version_cmake NAMES rdkit-config-version.cmake PATHS ${_libdir} NO_DEFAULT_PATH )

  if ( version_cmake )
    include( ${version_cmake} )
    set( RDKit_PACKAGE_VERSION ${PACKAGE_VERSION} )
  endif()
  
endif()
