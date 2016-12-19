# find RDKit

if ( rdkit_FOUND )
  return()
endif()

if ( WIN32 )

  find_package( Boost QUIET )
  
  set( _rdkit_libdirs
    "${CMAKE_SOURCE_DIR}/../rdkit/lib"  # rdkit default install (intree)
    "C:/RDKit/lib"
    )

else()

  set( _rdkit_libdirs "${CMAKE_SOURCE_DIR}/../rdkit" "${RDBASE}/lib" "$ENV{RDBASE}/lib" "/usr/local/lib" )

endif()

if ( NOT RDBASE AND $ENV{RDBASE} )
  set( RDBASE, $ENV{RDBASE} )
endif()

set( _rdkit_incdirs
  "${CMAKE_SOURCE_DIR}/../rdkit/Code"
  "${CMAKE_SOURCE_DIR}/../rdkit/External"
  "${RDBASE}/Code"  
  "${RDBASE}/External"
  "/usr/local/include/rdkit" )

if ( NOT rdkit_FOUND )

  find_path( _include_dir GraphMol/RDKitBase.h HINTS
    ${_rdkit_incdirs}
    /usr/include/rdkit
    )

  if ( _include_dir )
    message( STATUS "###########################################" )
    message( STATUS "## Findrdkit: found by include_dir search: "  ${_include_dir} )
  else()
    return()
  endif()

  find_path( _inchi_inc_dir INCHI-API/inchi.h HINTS ${_rdkit_incdirs} )
  
  find_library( _fileparsers_lib NAMES FileParsers HINTS
    ${_include_dir}/../lib
    /usr/local/lib
    /usr/lib )

  if ( _fileparsers_lib )
    get_filename_component ( _libdir ${_fileparsers_lib} PATH )
  else()
    return()
  endif()

  # replace RDBASE
  get_filename_component ( RDBASE ${_libdir} DIRECTORY )

  set ( rdkit_FOUND TRUE )
  set ( RDKit_INCLUDE_DIRS ${_include_dir} )
  if ( _inchi_inc_dir )
    list( APPEND RDKit_INCLUDE_DIRS ${_inchi_inc_dir} )
  endif()

  set ( RDKit_LIBRARY_DIRS ${_libdir} )

  ## MolDraw2DSVG.h might be located on /usr/local/include/rdkit/, or $RDBASE/Code/GraphMol/MolDraw2D/
  ## depend on rdkit CMakefile set INSTALL INTREE OFF or ON (default)

  find_path( _moldraw2d_include NAMES "MolDraw2DSVG.h" PATHS "${_include_dir}/GraphMol/MolDraw2D" )
  if ( _moldraw2d_include )
    list ( APPEND RDKit_INCLUDE_DIRS ${_moldraw2d_include} )
  endif()

  find_library( CHEMREACTIONS_LIB  NAMES ChemReactions  HINTS ${_libdir} )    
  find_library( DATASTRUCTS_LIB    NAMES DataStructs    HINTS ${_libdir} )
  find_library( DEPICTOR_LIB       NAMES Depictor       HINTS ${_libdir} )
  find_library( DESCRIPTORS_LIB    NAMES Descriptors    HINTS ${_libdir} )
  find_library( FINGERPRINTS_LIB   NAMES Fingerprints   HINTS ${_libdir} )  
  find_library( GRAPHMOL_LIB       NAMES GraphMol       HINTS ${_libdir} )
  find_library( MOLDRAW2D_LIB      NAMES MolDraw2D      HINTS ${_libdir} )
  find_library( PARTIALCHARGES_LIB NAMES PartialCharges HINTS ${_libdir} )
  find_library( RDGEOMETRYLIB_LIB  NAMES RDGeometryLib  HINTS ${_libdir} )
  find_library( RDGENERAL_LIB      NAMES RDGeneral      HINTS ${_libdir} )
  find_library( SMILESPARSE_LIB    NAMES SmilesParse    HINTS ${_libdir} )
  find_library( SUBSTRUCTMATCH_LIB NAMES SubstructMatch HINTS ${_libdir} )
  find_library( SUBGRAPHS_LIB      NAMES Subgraphs      HINTS ${_libdir} )
  find_library( INCHI_LIB          NAMES Inchi          HINTS ${_libdir} )
  find_library( RDINCHILIB_LIB     NAMES RDInchiLib     HINTS ${_libdir} )

  add_library( ChemReactions  SHARED IMPORTED )
  add_library( DataStructs    SHARED IMPORTED )
  add_library( Depictor       SHARED IMPORTED )
  add_library( Descriptors    SHARED IMPORTED )
  add_library( FileParsers    SHARED IMPORTED )
  add_library( Fingerprints   SHARED IMPORTED )
  add_library( GraphMol       SHARED IMPORTED )
  add_library( MolDraw2D      SHARED IMPORTED )
  add_library( PartialCharges SHARED IMPORTED )
  add_library( RDGeneral      SHARED IMPORTED )
  add_library( RDGeometryLib  SHARED IMPORTED )
  add_library( SmilesParse    SHARED IMPORTED )
  add_library( SubstructMatch SHARED IMPORTED )
  add_library( Subgraphs      SHARED IMPORTED )
  add_library( Inchi          SHARED IMPORTED )
  add_library( RDInchiLib     SHARED IMPORTED )  

  set_target_properties( ChemReactions  PROPERTIES IMPORTED_LOCATION ${CHEMREACTIONS_LIB} )
  set_target_properties( DataStructs    PROPERTIES IMPORTED_LOCATION ${DATASTRUCTS_LIB} )  
  set_target_properties( Depictor       PROPERTIES IMPORTED_LOCATION ${DEPICTOR_LIB} )
  set_target_properties( Descriptors    PROPERTIES IMPORTED_LOCATION ${DESCRIPTORS_LIB} )
  set_target_properties( FileParsers    PROPERTIES IMPORTED_LOCATION ${_fileparsers_lib} )
  set_target_properties( GraphMol       PROPERTIES IMPORTED_LOCATION ${GRAPHMOL_LIB} )
  set_target_properties( MolDraw2D      PROPERTIES IMPORTED_LOCATION ${MOLDRAW2D_LIB} )
  set_target_properties( PartialCharges PROPERTIES IMPORTED_LOCATION ${PARTIALCHARGES_LIB} )
  set_target_properties( RDGeneral      PROPERTIES IMPORTED_LOCATION ${RDGENERAL_LIB} )
  set_target_properties( RDGeometryLib  PROPERTIES IMPORTED_LOCATION ${RDGEOMETRYLIB_LIB} )
  set_target_properties( SmilesParse    PROPERTIES IMPORTED_LOCATION ${SMILESPARSE_LIB} )
  set_target_properties( SubstructMatch PROPERTIES IMPORTED_LOCATION ${SUBSTRUCTMATCH_LIB} )
  set_target_properties( Subgraphs      PROPERTIES IMPORTED_LOCATION ${SUBGRAPHS_LIB} )
  set_target_properties( Inchi          PROPERTIES IMPORTED_LOCATION ${INCHI_LIB} )
  set_target_properties( RDInchiLib     PROPERTIES IMPORTED_LOCATION ${RDINCHILIB_LIB} )

  find_file( version_cmake NAMES rdkit-config-version.cmake PATHS ${RDKit_LIBRARY_DIRS} NO_DEFAULT_PATH )
  if ( version_cmake )
    include( ${version_cmake} )
    set( RDKit_PACKAGE_VERSION ${PACKAGE_VERSION} )
  endif()

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
  MolDraw2D
  ChemReactions
  Inchi
  RDInchiLib
  )


