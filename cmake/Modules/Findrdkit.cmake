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

set ( rdkit_FOUND TRUE )
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
find_library( FINGERPRINTS_LIB NAMES FingerPrints HINTS ${_libdir} )
find_library( FINGERPRINTS_DEBUG_LIB NAMES FingerPrints${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT FINGERPRINTS_LIB )
  message( FATAL_ERROR "FINGERPRINTS not found" )
endif()
find_library( GRAPHMOL_LIB NAMES GraphMol HINTS ${_libdir} )
find_library( GRAPHMOL_DEBUG_LIB NAMES GraphMol${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )

find_library( MOLDRAW2D_LIB NAMES MolDraw2D HINTS ${_libdir} )
find_library( MOLDRAW2D_DEBUG_LIB NAMES MolDraw2D${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )

find_library( PARTIALCHARGES_LIB NAMES PartialCharges HINTS ${_libdir} )
find_library( PARTIALCHARGES_DEBUG_LIB NAMES PartialCharges${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )

find_library( SMILESPARSE_LIB NAMES SmilesParse HINTS ${_libdir} )
find_library( SMILESPARSE_DEBUG_LIB NAMES SmilesParse${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )

find_library( SUBSTRUCTMATCH_LIB NAMES SubstructMatch HINTS ${_libdir} )
find_library( SUBSTRUCTMATCH_DEBUG_LIB NAMES SubstructMatch${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )

find_library( CHEMTRANSFORMS_LIB NAMES ChemTransforms HINTS ${_libdir} )
find_library( CHEMTRANSFORMS_DEBUG_LIB NAMES ChemTransforms${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )

find_library( SUBGRAPHS_LIB NAMES Subgraphs HINTS ${_libdir} )
find_library( SUBGRAPHS_DEBUG_LIB NAMES Subgraphs${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )

find_library( MOLTRANSFORMS_LIB NAMES MolTransforms HINTS ${_libdir} )
find_library( MOLTRANSFORMS_DEBUG_LIB NAMES MolTransforms${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )

find_library( RDGEOMETRY_LIB NAMES RDGeometryLib HINTS ${_libdir} )
find_library( RDGEOMETRY_DEBUG_LIB NAMES RDGeometryLib${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )

find_library( RDGENERAL_LIB NAMES RDGeneral HINTS ${_libdir} )
find_library( RDGENERAL_DEBUG_LIB NAMES RDGeneral${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )

find_library( INCHI_LIB NAMES Inchi HINTS ${_libdir} )
find_library( INCHI_DEBUG_LIB NAMES Inchi${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )

find_library( RDINCHILIB_LIB NAMES RDInchiLib HINTS ${_libdir} )
find_library( RDINCHILIB_DEBUG_LIB NAMES RDInchiLib${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )

if ( WIN32 )
  set_target_properties( ChemReactions	PROPERTIES  IMPORTED_IMPLIB ${CHEMREACTIONS_LIB}  IMPORTED_IMPLIB_DEBUG ${CHEMREACTIONS_DEBUG_LIB} )
  set_target_properties( DataStructs	PROPERTIES  IMPORTED_IMPLIB ${DATASTRUCTS_LIB}	  IMPORTED_IMPLIB_DEBUG ${DATASTRUCTS_DEBUG_LIB} )
  set_target_properties( Depictor	PROPERTIES  IMPORTED_IMPLIB ${DEPICTOR_LIB}	  IMPORTED_IMPLIB_DEBUG ${DEPICTOR_DEBUG_LIB} )
  set_target_properties( Descriptors	PROPERTIES  IMPORTED_IMPLIB ${DESCRIPTORS_LIB}	  IMPORTED_IMPLIB_DEBUG ${DESCRIPTORS_DEBUG_LIB} )
  set_target_properties( EigenSolvers	PROPERTIES  IMPORTED_IMPLIB ${EIGENSOLVERS_LIB}	  IMPORTED_IMPLIB_DEBUG ${EIGENSOLVERS_DEBUG_LIB} )
  set_target_properties( FileParsers	PROPERTIES  IMPORTED_IMPLIB ${FILEPARSERS_LIB}	  IMPORTED_IMPLIB_DEBUG ${FILEPARSERS_DEBUG_LIB} )
  set_target_properties( Fingerprints	PROPERTIES  IMPORTED_IMPLIB ${FINGERPRINTS_LIB}	  IMPORTED_IMPLIB_DEBUG ${FINGERPRINTS_DEBUG_LIB} )
  set_target_properties( GraphMol	PROPERTIES  IMPORTED_IMPLIB ${GRAPHMOL_LIB}	  IMPORTED_IMPLIB_DEBUG ${GRAPHMOL_DEBUG_LIB} )
  set_target_properties( MolDraw2D	PROPERTIES  IMPORTED_IMPLIB ${MOLDRAW2D_LIB}	  IMPORTED_IMPLIB_DEBUG ${MOLDRAW2D_DEBUG_LIB} )
  set_target_properties( PartialCharges	PROPERTIES  IMPORTED_IMPLIB ${PARTIALCHARGES_LIB} IMPORTED_IMPLIB_DEBUG ${PARTIALCHARGES_DEBUG_LIB} )
  set_target_properties( SmilesParse	PROPERTIES  IMPORTED_IMPLIB ${SMILESPARSE_LIB}	  IMPORTED_IMPLIB_DEBUG ${SMILESPARSE_DEBUG_LIB} )
  set_target_properties( SubstructMatch	PROPERTIES  IMPORTED_IMPLIB ${SUBSTRUCTMATCH_LIB} IMPORTED_IMPLIB_DEBUG ${SUBSTRUCTMATCH_DEBUG_LIB} )
  set_target_properties( ChemTransforms	PROPERTIES  IMPORTED_IMPLIB ${CHEMTRANSFORMS_LIB} IMPORTED_IMPLIB_DEBUG ${CHEMTRANSFORMS_DEBUG_LIB} )
  set_target_properties( Subgraphs	PROPERTIES  IMPORTED_IMPLIB ${SUBGRAPHS_LIB}	  IMPORTED_IMPLIB_DEBUG ${SUBGRAPHS_DEBUG_LIB} )
  set_target_properties( MolTransforms	PROPERTIES  IMPORTED_IMPLIB ${MOLTRANSFORMS_LIB}  IMPORTED_IMPLIB_DEBUG ${MOLTRANSFORMS_DEBUG_LIB} )
  set_target_properties( RDGeometryLib	PROPERTIES  IMPORTED_IMPLIB ${RDGEOMETRYLIB_LIB}  IMPORTED_IMPLIB_DEBUG ${RDGEOMETRYLIB_DEBUG_LIB} )
  set_target_properties( RDGeneral	PROPERTIES  IMPORTED_IMPLIB ${RDGENERAL_LIB}	  IMPORTED_IMPLIB_DEBUG ${RDGENERAL_DEBUG_LIB} )
  set_target_properties( Inchi		PROPERTIES  IMPORTED_IMPLIB ${INCHI_LIB}	  IMPORTED_IMPLIB_DEBUG ${INCHI_DEBUG_LIB} )
  set_target_properties( RDInchiLib	PROPERTIES  IMPORTED_IMPLIB ${RDINCHILIB_LIB}	  IMPORTED_IMPLIB_DEBUG ${RDINCHILIB_DEBUG_LIB} )
else()
  if ( APPLE )
    set( SO "dylib" )
  else()
    set( SO "so" )
  endif()
  set_target_properties( ChemReactions	PROPERTIES IMPORTED_LOCATION ${_libdir}/libChemReactions.${SO} )
  set_target_properties( DataStructs	PROPERTIES IMPORTED_LOCATION ${_libdir}/libDataStructs.${SO} )
  set_target_properties( Depictor	PROPERTIES IMPORTED_LOCATION ${_libdir}/libDepictor.${SO} )
  set_target_properties( Descriptors	PROPERTIES IMPORTED_LOCATION ${_libdir}/libDescriptors.${SO} )
  set_target_properties( EigenSolvers	PROPERTIES IMPORTED_LOCATION ${_libdir}/libEigenSolvers.${SO} )
  set_target_properties( FileParsers	PROPERTIES IMPORTED_LOCATION ${_libdir}/libFileParsers.${SO} )
  set_target_properties( Fingerprints	PROPERTIES IMPORTED_LOCATION ${_libdir}/libFingerprints.${SO} )
  set_target_properties( GraphMol	PROPERTIES IMPORTED_LOCATION ${_libdir}/libGraphMol.${SO} )       
  set_target_properties( MolDraw2D	PROPERTIES IMPORTED_LOCATION ${_libdir}/libMolDraw2D.${SO} )
  set_target_properties( PartialCharges	PROPERTIES IMPORTED_LOCATION ${_libdir}/libPartialCharges.${SO} )
  set_target_properties( SmilesParse	PROPERTIES IMPORTED_LOCATION ${_libdir}/libSmilesParse.${SO} )
  set_target_properties( SubstructMatch	PROPERTIES IMPORTED_LOCATION ${_libdir}/libSubstructMatch.${SO} )
  set_target_properties( ChemTransforms	PROPERTIES IMPORTED_LOCATION ${_libdir}/libChemTransforms.${SO} )
  set_target_properties( Subgraphs	PROPERTIES IMPORTED_LOCATION ${_libdir}/libSubgraphs.${SO} )
  set_target_properties( MolTransforms	PROPERTIES IMPORTED_LOCATION ${_libdir}/libMolTransforms.${SO} )
  set_target_properties( RDGeometryLib	PROPERTIES IMPORTED_LOCATION ${_libdir}/libRDGeometryLib.${SO} )
  set_target_properties( RDGeneral	PROPERTIES IMPORTED_LOCATION ${_libdir}/libRDGeneral.${SO} )
  set_target_properties( Inchi		PROPERTIES IMPORTED_LOCATION ${_libdir}/libInchi.${SO} )
  set_target_properties( RDInchiLib	PROPERTIES IMPORTED_LOCATION ${_libdir}/libRDInchiLib.${SO} )    
endif()




