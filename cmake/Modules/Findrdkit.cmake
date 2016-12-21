# find RDKit

if ( rdkit_FOUND )
  return()
endif()

set ( rdkit "rdkit-NOTFOUND" )

if ( WIN32 )

  set( _rdkit_libdirs
    "${CMAKE_SOURCE_DIR}/../rdkit/lib"  # rdkit default install (intree)
    "C:/RDKit/lib"
    )

else()

  set( _rdkit_libdirs "${CMAKE_SOURCE_DIR}/../rdkit" "${RDBASE}/lib" "$ENV{RDBASE}/lib" "/usr/local/lib" )

endif()

set( _rdkit_incdirs
  "${CMAKE_SOURCE_DIR}/../rdkit/Code"
  "${CMAKE_SOURCE_DIR}/../rdkit/External"
  "${RDBASE}/Code"  
  "${RDBASE}/External"
  "/usr/local/include/rdkit" )

find_path( _include_dir GraphMol/RDKitBase.h HINTS ${_rdkit_incdirs} )

if ( NOT _include_dir )
  return()
endif()

find_path( _inchi_inc_dir INCHI-API/inchi.h HINTS ${_rdkit_incdirs} )

find_library( _fileparsers_lib NAMES FileParsers HINTS
  ${_rdkit_libdirs}
  ${_include_dir}/../lib
  /usr/local/lib
  /usr/lib )

if ( NOT _fileparsers_lib )
  return()
endif()

get_filename_component ( _libdir ${_fileparsers_lib} PATH )
get_filename_component ( RDBASE ${_libdir} DIRECTORY )
set ( RDKit_LIBRARY_DIRS ${_libdir} )

set ( RDKit_INCLUDE_DIRS ${_include_dir} )
if ( _inchi_inc_dir )
  list( APPEND RDKit_INCLUDE_DIRS ${_inchi_inc_dir} )
endif()

find_path( _moldraw2d_include NAMES "MolDraw2DSVG.h" PATHS "${_include_dir}/GraphMol/MolDraw2D" )

if ( _moldraw2d_include )
  list ( APPEND RDKit_INCLUDE_DIRS ${_moldraw2d_include} )
endif()

find_file( version_cmake NAMES "rdkit-config-version.cmake" PATHS ${_libdir} NO_DEFAULT_PATH )
if ( version_cmake )
  include( ${version_cmake} )
  set( RDKit_PACKAGE_VERSION ${PACKAGE_VERSION} )
endif()

set( CHEMREACTIONS_LIB "CHEMREACTIONS_LIB-NOTFOUND" )
set( CHEMREACTIONS_DEBUG_LIB "CHEMREACTIONS_DEBUG_LIB-NOTFOUND" )

set( DATASTRUCTS_LIB "DATASTRUCTS_LIB-NOTFOUND" )
set( DATASTRUCTS_DEBUG_LIB "DATASTRUCTS_DEBUG_LIB-NOTFOUND" )

set( DEPICTOR_LIB "DEPICTOR_LIB-NOTFOUND" )
set( DEPICTOR_DEBUG_LIB "DEPICTOR_DEBUG_LIB-NOTFOUND" )

set( DESCRIPTORS_LIB "DESCRIPTORS_LIB-NOTFOUND" )
set( DESCRIPTORS_DEBUG_LIB "DESCRIPTORS_DEBUG_LIB-NOTFOUND" )

set( EIGENSOLVERS_LIB "EIGENSOLVERS_LIB-NOTFOUND" )
set( EIGENSOLVERS_DEBUG_LIB "EIGENSOLVERS_DEBUG_LIB-NOTFOUND" )

set( FILEPARSERS_LIB "FILEPARSERS_LIB-NOTFOUND" )
set( FILEPARSERS_DEBUG_LIB "FILEPARSERS_DEBUG_LIB-NOTFOUND" )

set( FINGERPRINTS_LIB "FINGERPRINTS_LIB-NOTFOUND" )
set( FINGERPRINTS_DEBUG_LIB "FINGERPRINTS_DEBUG_LIB-NOTFOUND" )

set( GRAPHMOL_LIB "GRAPHMOL_LIB-NOTFOUND" )
set( GRAPHMOL_DEBUG_LIB "GRAPHMOL_DEBUG_LIB-NOTFOUND" )

set( MOLDRAW2D_LIB "MOLDRAW2D_LIB-NOTFOUND" )
set( MOLDRAW2D_DEBUG_LIB "MOLDRAW2D_DEBUG_LIB-NOTFOUND" )

set( PARTIALCHARGES_LIB "PARTIALCHARGES_LIB-NOTFOUND" )
set( PARTIALCHARGES_DEBUG_LIB "PARTIALCHARGES_DEBUG_LIB-NOTFOUND" )

set( SMILESPARSE_LIB "SMILESPARSE_LIB-NOTFOUND" )
set( SMILESPARSE_DEBUG_LIB "SMILESPARSE_DEBUG_LIB-NOTFOUND" )

set( SUBSTRUCTMATCH_LIB "SUBSTRUCTMATCH_LIB-NOTFOUND" )
set( SUBSTRUCTMATCH_DEBUG_LIB "SUBSTRUCTMATCH_DEBUG_LIB-NOTFOUND" )

set( CHEMTRANSFORMS_LIB "CHEMTRANSFORMS_LIB-NOTFOUND" )
set( CHEMTRANSFORMS_DEBUG_LIB "CHEMTRANSFORMS_DEBUG_LIB-NOTFOUND" )

set( SUBGRAPHS_LIB "SUBGRAPHS_LIB-NOTFOUND" )
set( SUBGRAPHS_DEBUG_LIB "SUBGRAPHS_DEBUG_LIB-NOTFOUND" )

set( MOLTRANSFORMS_LIB "MOLTRANSFORMS_LIB-NOTFOUND" )
set( MOLTRANSFORMS_DEBUG_LIB "MOLTRANSFORMS_DEBUG_LIB-NOTFOUND" )

set( RDGEOMETRYLIB_LIB "RDGEOMETRYLIB_LIB-NOTFOUND" )
set( RDGEOMETRYLIB_DEBUG_LIB "RDGEOMETRYLIB_DEBUG_LIB-NOTFOUND" )

set( RDGENERAL_LIB "RDGENERAL_LIB-NOTFOUND" )
set( RDGENERAL_DEBUG_LIB "RDGENERAL_DEBUG_LIB-NOTFOUND" )

set( INCHI_LIB "INCHI_LIB-NOTFOUND" )
set( INCHI_DEBUG_LIB "INCHI_DEBUG_LIB-NOTFOUND" )

set( RDINCHILIB_LIB "RDINCHILIB_LIB-NOTFOUND" )
set( RDINCHILIB_DEBUG_LIB "RDINCHILIB_DEBUG_LIB-NOTFOUND" )

if ( WIN32 )
  add_library( ChemReactions	STATIC IMPORTED )
  add_library( DataStructs	STATIC IMPORTED )
  add_library( Depictor		STATIC IMPORTED )
  add_library( Descriptors	STATIC IMPORTED )
  add_library( EigenSolvers	STATIC IMPORTED )
  add_library( FileParsers	STATIC IMPORTED )
  add_library( Fingerprints	STATIC IMPORTED )
  add_library( GraphMol		STATIC IMPORTED )
  add_library( MolDraw2D	STATIC IMPORTED )
  add_library( PartialCharges	STATIC IMPORTED )
  add_library( SmilesParse	STATIC IMPORTED )
  add_library( SubstructMatch	STATIC IMPORTED )
  add_library( ChemTransforms	STATIC IMPORTED )
  add_library( Subgraphs	STATIC IMPORTED )
  add_library( MolTransforms	STATIC IMPORTED )
  add_library( RDGeometryLib	STATIC IMPORTED )
  add_library( RDGeneral	STATIC IMPORTED )
  add_library( Inchi		STATIC IMPORTED )
  add_library( RDInchiLib	STATIC IMPORTED )
else()
  add_library( ChemReactions	SHARED IMPORTED )
  add_library( DataStructs	SHARED IMPORTED )
  add_library( Depictor		SHARED IMPORTED )
  add_library( Descriptors	SHARED IMPORTED )
  add_library( EigenSolvers	SHARED IMPORTED )
  add_library( FileParsers	SHARED IMPORTED )
  add_library( Fingerprints	SHARED IMPORTED )
  add_library( GraphMol		SHARED IMPORTED )
  add_library( MolDraw2D	SHARED IMPORTED )
  add_library( PartialCharges	SHARED IMPORTED )
  add_library( SmilesParse	SHARED IMPORTED )
  add_library( SubstructMatch	SHARED IMPORTED )
  add_library( ChemTransforms	SHARED IMPORTED )
  add_library( Subgraphs	SHARED IMPORTED )
  add_library( MolTransforms	SHARED IMPORTED )
  add_library( RDGeometryLib	SHARED IMPORTED )
  add_library( RDGeneral	SHARED IMPORTED )
  add_library( Inchi		SHARED IMPORTED )
  add_library( RDInchiLib	SHARED IMPORTED )
endif()

set ( RDKit_LIBRARIES
  ChemReactions
  DataStructs
  Depictor
  Descriptors  
  EigenSolvers
  FileParsers
  Fingerprints
  GraphMol
  MolDraw2D
  PartialCharges
  SmilesParse
  SubstructMatch
  ChemTransforms
  Subgraphs
  MolTransforms
  RDGeometryLib
  RDGeneral
  Inchi
  RDInchiLib
  )

find_library( CHEMREACTIONS_LIB NAMES ChemReactions HINTS ${_libdir} )
find_library( CHEMREACTIONS_DEBUG_LIB NAMES ChemReactions${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT CHEMREACTIONS_LIB )
  message( FATAL_ERROR "ChemReactions not found" )
endif()

find_library( DATASTRUCTS_LIB NAMES DataStructs HINTS ${_libdir} )
find_library( DATASTRUCTS_DEBUG_LIB NAMES DataStructs${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT DATASTRUCTS_LIB )
  message( FATAL_ERROR "DataStructs not found" )
endif()

find_library( DEPICTOR_LIB NAMES Depictor HINTS ${_libdir} )
find_library( DEPICTOR_DEBUG_LIB NAMES Depictor${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT DEPICTOR_LIB )
  message( FATAL_ERROR "DEPICTOR not found" )
endif()

find_library( DESCRIPTORS_LIB NAMES Descriptors HINTS ${_libdir} )
find_library( DESCRIPTORS_DEBUG_LIB NAMES Descriptors${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT DESCRIPTORS_LIB )
  message( FATAL_ERROR "DESCRIPTORS not found" )
endif()

find_library( EIGENSOLVERS_LIB NAMES EigenSolvers HINTS ${_libdir} )
find_library( EIGENSOLVERS_DEBUG_LIB NAMES EigenSolvers${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT EIGENSOLVERS_LIB )
  message( FATAL_ERROR "EIGENSOLVERS not found" )
endif()

find_library( FILEPARSERS_LIB NAMES FileParsers HINTS ${_libdir} )
find_library( FILEPARSERS_DEBUG_LIB NAMES FileParsers${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT FILEPARSERS_LIB )
  message( FATAL_ERROR "FILEPARSERS not found" )
endif()
if ( NOT FILEPARSERS_DEBUG_LIB )
  message( FATAL_ERROR "FILEPARSERS_DEBUG not found" )
endif()

find_library( FINGERPRINTS_LIB NAMES Fingerprints HINTS ${_libdir} )
find_library( FINGERPRINTS_DEBUG_LIB NAMES Fingerprints${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT FINGERPRINTS_LIB )
  message( FATAL_ERROR "FINGERPRINTS not found" )
endif()

find_library( GRAPHMOL_LIB NAMES GraphMol HINTS ${_libdir} )
find_library( GRAPHMOL_DEBUG_LIB NAMES GraphMol${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT GRAPHMOL_LIB )
  message( FATAL_ERROR "GraphMol not found" )
endif()

find_library( MOLDRAW2D_LIB NAMES MolDraw2D HINTS ${_libdir} )
find_library( MOLDRAW2D_DEBUG_LIB NAMES MolDraw2D${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT MOLDRAW2D_LIB )
  message( FATAL_ERROR "MolDraw2D not found" )
endif()

find_library( PARTIALCHARGES_LIB NAMES PartialCharges HINTS ${_libdir} )
find_library( PARTIALCHARGES_DEBUG_LIB NAMES PartialCharges${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT PARTIALCHARGES_LIB )
  message( FATAL_ERROR "PartialCharges not found" )
endif()

find_library( SMILESPARSE_LIB NAMES SmilesParse HINTS ${_libdir} )
find_library( SMILESPARSE_DEBUG_LIB NAMES SmilesParse${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT SMILESPARSE_LIB )
  message( FATAL_ERROR "SmilesParse not found" )
endif()

find_library( SUBSTRUCTMATCH_LIB NAMES SubstructMatch HINTS ${_libdir} )
find_library( SUBSTRUCTMATCH_DEBUG_LIB NAMES SubstructMatch${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT SUBSTRUCTMATCH_LIB )
  message( FATAL_ERROR "SubstructMatch not found" )
endif()

find_library( CHEMTRANSFORMS_LIB NAMES ChemTransforms HINTS ${_libdir} )
find_library( CHEMTRANSFORMS_DEBUG_LIB NAMES ChemTransforms${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT CHEMTRANSFORMS_LIB )
  message( FATAL_ERROR "ChemTransforms not found" )
endif()

find_library( SUBGRAPHS_LIB NAMES Subgraphs HINTS ${_libdir} )
find_library( SUBGRAPHS_DEBUG_LIB NAMES Subgraphs${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT SUBGRAPHS_LIB )
  message( FATAL_ERROR "Subgraphs not found" )
endif()

find_library( MOLTRANSFORMS_LIB NAMES MolTransforms HINTS ${_libdir} )
find_library( MOLTRANSFORMS_DEBUG_LIB NAMES MolTransforms${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT MOLTRANSFORMS_LIB )
  message( FATAL_ERROR "MolTransforms not found" )
endif()

find_library( RDGEOMETRYLIB_LIB NAMES RDGeometryLib HINTS ${_libdir} )
find_library( RDGEOMETRYLIB_DEBUG_LIB NAMES RDGeometryLib${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT RDGEOMETRYLIB_LIB )
  message( FATAL_ERROR "RDGeometryLib not found" )
endif()

find_library( RDGENERAL_LIB NAMES RDGeneral HINTS ${_libdir} )
find_library( RDGENERAL_DEBUG_LIB NAMES RDGeneral${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT RDGENERAL_LIB )
  message( FATAL_ERROR "RDGeneral not found" )
endif()

find_library( INCHI_LIB NAMES Inchi HINTS ${_libdir} )
find_library( INCHI_DEBUG_LIB NAMES Inchi${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT INCHI_LIB )
  message( FATAL_ERROR "Inchi not found" )
endif()

find_library( RDINCHILIB_LIB NAMES RDInchiLib HINTS ${_libdir} )
find_library( RDINCHILIB_DEBUG_LIB NAMES RDInchiLib${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT RDINCHILIB_LIB )
  message( FATAL_ERROR "RDInchiLib not found" )
endif()

if ( WIN32 )
  set_target_properties( ChemReactions	PROPERTIES  IMPORTED_LOCATION ${CHEMREACTIONS_LIB}  IMPORTED_LOCATION_DEBUG ${CHEMREACTIONS_DEBUG_LIB} )
  set_target_properties( DataStructs	PROPERTIES  IMPORTED_LOCATION ${DATASTRUCTS_LIB}    IMPORTED_LOCATION_DEBUG ${DATASTRUCTS_DEBUG_LIB} )
  set_target_properties( Depictor	PROPERTIES  IMPORTED_LOCATION ${DEPICTOR_LIB}	    IMPORTED_LOCATION_DEBUG ${DEPICTOR_DEBUG_LIB} )
  set_target_properties( Descriptors	PROPERTIES  IMPORTED_LOCATION ${DESCRIPTORS_LIB}    IMPORTED_LOCATION_DEBUG ${DESCRIPTORS_DEBUG_LIB} )
  set_target_properties( EigenSolvers	PROPERTIES  IMPORTED_LOCATION ${EIGENSOLVERS_LIB}   IMPORTED_LOCATION_DEBUG ${EIGENSOLVERS_DEBUG_LIB} )
  set_target_properties( FileParsers	PROPERTIES  IMPORTED_LOCATION ${FILEPARSERS_LIB}    IMPORTED_LOCATION_DEBUG ${FILEPARSERS_DEBUG_LIB} )
  set_target_properties( Fingerprints	PROPERTIES  IMPORTED_LOCATION ${FINGERPRINTS_LIB}   IMPORTED_LOCATION_DEBUG ${FINGERPRINTS_DEBUG_LIB} )
  set_target_properties( GraphMol	PROPERTIES  IMPORTED_LOCATION ${GRAPHMOL_LIB}	    IMPORTED_LOCATION_DEBUG ${GRAPHMOL_DEBUG_LIB} )
  set_target_properties( MolDraw2D	PROPERTIES  IMPORTED_LOCATION ${MOLDRAW2D_LIB}	    IMPORTED_LOCATION_DEBUG ${MOLDRAW2D_DEBUG_LIB} )
  set_target_properties( PartialCharges	PROPERTIES  IMPORTED_LOCATION ${PARTIALCHARGES_LIB} IMPORTED_LOCATION_DEBUG ${PARTIALCHARGES_DEBUG_LIB} )
  set_target_properties( SmilesParse	PROPERTIES  IMPORTED_LOCATION ${SMILESPARSE_LIB}    IMPORTED_LOCATION_DEBUG ${SMILESPARSE_DEBUG_LIB} )
  set_target_properties( SubstructMatch	PROPERTIES  IMPORTED_LOCATION ${SUBSTRUCTMATCH_LIB} IMPORTED_LOCATION_DEBUG ${SUBSTRUCTMATCH_DEBUG_LIB} )
  set_target_properties( ChemTransforms	PROPERTIES  IMPORTED_LOCATION ${CHEMTRANSFORMS_LIB} IMPORTED_LOCATION_DEBUG ${CHEMTRANSFORMS_DEBUG_LIB} )
  set_target_properties( Subgraphs	PROPERTIES  IMPORTED_LOCATION ${SUBGRAPHS_LIB}	    IMPORTED_LOCATION_DEBUG ${SUBGRAPHS_DEBUG_LIB} )
  set_target_properties( MolTransforms	PROPERTIES  IMPORTED_LOCATION ${MOLTRANSFORMS_LIB}  IMPORTED_LOCATION_DEBUG ${MOLTRANSFORMS_DEBUG_LIB} )
  set_target_properties( RDGeometryLib	PROPERTIES  IMPORTED_LOCATION ${RDGEOMETRYLIB_LIB}  IMPORTED_LOCATION_DEBUG ${RDGEOMETRYLIB_DEBUG_LIB} )
  set_target_properties( RDGeneral	PROPERTIES  IMPORTED_LOCATION ${RDGENERAL_LIB}	    IMPORTED_LOCATION_DEBUG ${RDGENERAL_DEBUG_LIB} )
  set_target_properties( Inchi		PROPERTIES  IMPORTED_LOCATION ${INCHI_LIB}	    IMPORTED_LOCATION_DEBUG ${INCHI_DEBUG_LIB} )
  set_target_properties( RDInchiLib	PROPERTIES  IMPORTED_LOCATION ${RDINCHILIB_LIB}	    IMPORTED_LOCATION_DEBUG ${RDINCHILIB_DEBUG_LIB} )
else()
  set_target_properties( ChemReactions	PROPERTIES IMPORTED_LOCATION ${CHEMREACTIONS_LIB} )
  set_target_properties( DataStructs	PROPERTIES IMPORTED_LOCATION ${DATASTRUCTS_LIB} )
  set_target_properties( Depictor	PROPERTIES IMPORTED_LOCATION ${DEPICTOR_LIB} )
  set_target_properties( Descriptors	PROPERTIES IMPORTED_LOCATION ${DESCRIPTORS_LIB} )
  set_target_properties( EigenSolvers	PROPERTIES IMPORTED_LOCATION ${EIGENSOLVERS_LIB} )
  set_target_properties( FileParsers	PROPERTIES IMPORTED_LOCATION ${FILEPARSERS_LIB} )
  set_target_properties( Fingerprints	PROPERTIES IMPORTED_LOCATION ${FINGERPRINTS_LIB} )
  set_target_properties( GraphMol	PROPERTIES IMPORTED_LOCATION ${GRAPHMOL_LIB} )
  set_target_properties( MolDraw2D	PROPERTIES IMPORTED_LOCATION ${MOLDRAW2D_LIB} )
  set_target_properties( PartialCharges	PROPERTIES IMPORTED_LOCATION ${PARTIALCHARGES_LIB} )
  set_target_properties( SmilesParse	PROPERTIES IMPORTED_LOCATION ${SMILESPARSE_LIB} )
  set_target_properties( SubstructMatch	PROPERTIES IMPORTED_LOCATION ${SUBSTRUCTMATCH_LIB} )
  set_target_properties( ChemTransforms	PROPERTIES IMPORTED_LOCATION ${CHEMTRANSFORMS_LIB} )
  set_target_properties( Subgraphs	PROPERTIES IMPORTED_LOCATION ${SUBGRAPHS_LIB} )
  set_target_properties( MolTransforms	PROPERTIES IMPORTED_LOCATION ${MOLTRANSFORMS_LIB} )
  set_target_properties( RDGeometryLib	PROPERTIES IMPORTED_LOCATION ${RDGEOMETRYLIB_LIB} )
  set_target_properties( RDGeneral	PROPERTIES IMPORTED_LOCATION ${RDGENERAL_LIB} )
  set_target_properties( Inchi		PROPERTIES IMPORTED_LOCATION ${INCHI_LIB} )
  set_target_properties( RDInchiLib	PROPERTIES IMPORTED_LOCATION ${RDINCHILIB_LIB} )
endif()

set ( rdkit TRUE )
set ( rdkit_FOUND TRUE )

