# find RDKit

if ( rdkit_FOUND )
  return()
endif()

set ( rdkit "rdkit-NOTFOUND" )
set( _rdkit_libdirs "/usr/local/lib" "${CMAKE_SOURCE_DIR}/../rdkit/lib" "${RDBASE}/lib" "$ENV{RDBASE}/lib" )

set( _rdkit_incdirs
  "/usr/local/include/rdkit"
  "${CMAKE_SOURCE_DIR}/../rdkit/Code"
  "${CMAKE_SOURCE_DIR}/../rdkit/External"
  "${RDBASE}/Code"  
  "${RDBASE}/External"
  )

find_path( _include_dir GraphMol/RDKitBase.h HINTS ${_rdkit_incdirs} )

if ( NOT _include_dir )
  return()
endif()

find_path( _inchi_inc_dir INCHI-API/inchi.h HINTS ${_rdkit_incdirs} )

find_library( _fileparsers_lib NAMES RDKitFileParsers HINTS
  ${_rdkit_libdirs}
  ${_include_dir}/../lib
  /usr/local/lib
  /usr/lib )

message( STATUS "################ " ${_fileparsers_lib} )

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

if ( WIN32 )
  add_library( RDKitChemReactions	STATIC IMPORTED )
  add_library( RDKitDataStructs		STATIC IMPORTED )
  add_library( RDKitDepictor		STATIC IMPORTED )
  add_library( RDKitDescriptors		STATIC IMPORTED )
  add_library( RDKitEigenSolvers	STATIC IMPORTED )
  add_library( RDKitFileParsers		STATIC IMPORTED )
  add_library( RDKitFingerprints	STATIC IMPORTED )
  add_library( RDKitGraphMol		STATIC IMPORTED )
  add_library( RDKitMolDraw2D		STATIC IMPORTED )
  add_library( RDKitPartialCharges	STATIC IMPORTED )
  add_library( RDKitSmilesParse		STATIC IMPORTED )
  add_library( RDKitSubstructMatch	STATIC IMPORTED )
  add_library( RDKitChemTransforms	STATIC IMPORTED )
  add_library( RDKitSubgraphs		STATIC IMPORTED )
  add_library( RDKitMolTransforms	STATIC IMPORTED )
  add_library( RDKitRDGeometryLib	STATIC IMPORTED )
  add_library( RDKitRDGeneral		STATIC IMPORTED )
  add_library( RDKitInchi		STATIC IMPORTED )
  add_library( RDKitRDInchiLib		STATIC IMPORTED )
  add_library( RDKitFilterCatalog	STATIC IMPORTED )
  add_library( RDKitCatalogs		STATIC IMPORTED )  
else()
  add_library( RDKitChemReactions	SHARED IMPORTED )
  add_library( RDKitDataStructs		SHARED IMPORTED )
  add_library( RDKitDepictor		SHARED IMPORTED )
  add_library( RDKitDescriptors		SHARED IMPORTED )
  add_library( RDKitEigenSolvers	SHARED IMPORTED )
  add_library( RDKitFileParsers		SHARED IMPORTED )
  add_library( RDKitFingerprints	SHARED IMPORTED )
  add_library( RDKitGraphMol		SHARED IMPORTED )
  add_library( RDKitMolDraw2D		SHARED IMPORTED )
  add_library( RDKitPartialCharges	SHARED IMPORTED )
  add_library( RDKitSmilesParse		SHARED IMPORTED )
  add_library( RDKitSubstructMatch	SHARED IMPORTED )
  add_library( RDKitChemTransforms	SHARED IMPORTED )
  add_library( RDKitSubgraphs		SHARED IMPORTED )
  add_library( RDKitMolTransforms	SHARED IMPORTED )
  add_library( RDKitRDGeometryLib	SHARED IMPORTED )
  add_library( RDKitRDGeneral		SHARED IMPORTED )
  add_library( RDKitInchi		SHARED IMPORTED )
  add_library( RDKitRDInchiLib		SHARED IMPORTED )
  add_library( RDKitFilterCatalog	SHARED IMPORTED )
  add_library( RDKitCatalogs		SHARED IMPORTED )
endif()

find_library( CHEMREACTIONS_LIB NAMES RDKitChemReactions HINTS ${_libdir} )
find_library( CHEMREACTIONS_DEBUG_LIB NAMES RDKitChemReactions${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT CHEMREACTIONS_LIB )
  message( FATAL_ERROR "${CHEMREACTIONS_LIB}" )
endif()

find_library( DATASTRUCTS_LIB NAMES RDKitDataStructs HINTS ${_libdir} )
find_library( DATASTRUCTS_DEBUG_LIB NAMES RDKitDataStructs${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT DATASTRUCTS_LIB )
  message( FATAL_ERROR "${DATASTRUCTS_LIB}" )
endif()

find_library( DEPICTOR_LIB NAMES RDKitDepictor HINTS ${_libdir} )
find_library( DEPICTOR_DEBUG_LIB NAMES RDKitDepictor${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT DEPICTOR_LIB )
  message( FATAL_ERROR "${DEPICTOR_LIB}" )
endif()

find_library( DESCRIPTORS_LIB NAMES RDKitDescriptors HINTS ${_libdir} )
find_library( DESCRIPTORS_DEBUG_LIB NAMES RDKitDescriptors${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT DESCRIPTORS_LIB )
  message( FATAL_ERROR "${DESCRIPTORS_LIB}" )
endif()

find_library( EIGENSOLVERS_LIB NAMES RDKitEigenSolvers HINTS ${_libdir} )
find_library( EIGENSOLVERS_DEBUG_LIB NAMES RDKitEigenSolvers${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT EIGENSOLVERS_LIB )
  message( FATAL_ERROR "${EIGENSOLVERS_LIB}" )
endif()

find_library( FILEPARSERS_LIB NAMES RDKitFileParsers HINTS ${_libdir} )
find_library( FILEPARSERS_DEBUG_LIB NAMES RDKitFileParsers${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT FILEPARSERS_LIB )
  message( FATAL_ERROR "${FILEPARSERS_LIB}" )
endif()

find_library( FINGERPRINTS_LIB NAMES RDKitFingerprints HINTS ${_libdir} )
find_library( FINGERPRINTS_DEBUG_LIB NAMES RDKitFingerprints${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT FINGERPRINTS_LIB )
  message( FATAL_ERROR "${FINGERPRINTS_LIB}" )
endif()

find_library( GRAPHMOL_LIB NAMES RDKitGraphMol HINTS ${_libdir} )
find_library( GRAPHMOL_DEBUG_LIB NAMES RDKitGraphMol${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT GRAPHMOL_LIB )
  message( FATAL_ERROR "${GRAPHMOL_LIB}" )
endif()

find_library( MOLDRAW2D_LIB NAMES RDKitMolDraw2D HINTS ${_libdir} )
find_library( MOLDRAW2D_DEBUG_LIB NAMES RDKitMolDraw2D${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT MOLDRAW2D_LIB )
  message( FATAL_ERROR "${MOLDRAW2D_LIB}" )
endif()

find_library( PARTIALCHARGES_LIB NAMES RDKitPartialCharges HINTS ${_libdir} )
find_library( PARTIALCHARGES_DEBUG_LIB NAMES RDKitPartialCharges${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT PARTIALCHARGES_LIB )
  message( FATAL_ERROR "${PARTIALCHARGES_LIB}" )
endif()

find_library( SMILESPARSE_LIB NAMES RDKitSmilesParse HINTS ${_libdir} )
find_library( SMILESPARSE_DEBUG_LIB NAMES RDKitSmilesParse${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT SMILESPARSE_LIB )
  message( FATAL_ERROR "${SMILESPARSE_LIB}" )
endif()

find_library( SUBSTRUCTMATCH_LIB NAMES RDKitSubstructMatch HINTS ${_libdir} )
find_library( SUBSTRUCTMATCH_DEBUG_LIB NAMES RDKitSubstructMatch${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT SUBSTRUCTMATCH_LIB )
  message( FATAL_ERROR "${SUBSTRUCTMATCH_LIB}" )
endif()

find_library( CHEMTRANSFORMS_LIB NAMES RDKitChemTransforms HINTS ${_libdir} )
find_library( CHEMTRANSFORMS_DEBUG_LIB NAMES RDKitChemTransforms${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT CHEMTRANSFORMS_LIB )
  message( FATAL_ERROR "${CHEMTRANSFORMS_LIB}" )
endif()

find_library( SUBGRAPHS_LIB NAMES RDKitSubgraphs HINTS ${_libdir} )
find_library( SUBGRAPHS_DEBUG_LIB NAMES RDKitSubgraphs${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT SUBGRAPHS_LIB )
  message( FATAL_ERROR "${SUBGRAPHS_LIB}" )
endif()

find_library( MOLTRANSFORMS_LIB NAMES RDKitMolTransforms HINTS ${_libdir} )
find_library( MOLTRANSFORMS_DEBUG_LIB NAMES RDKitMolTransforms${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT MOLTRANSFORMS_LIB )
  message( FATAL_ERROR "${MOLTRANSFORMS_LIB}" )
endif()

find_library( RDGEOMETRYLIB_LIB NAMES RDKitRDGeometryLib HINTS ${_libdir} )
find_library( RDGEOMETRYLIB_DEBUG_LIB NAMES RDKitRDGeometryLib${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT RDGEOMETRYLIB_LIB )
  message( FATAL_ERROR "${RDGEOMETRYLIB_LIB}" )
endif()

find_library( RDGENERAL_LIB NAMES RDKitRDGeneral HINTS ${_libdir} )
find_library( RDGENERAL_DEBUG_LIB NAMES RDKitRDGeneral${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT RDGENERAL_LIB )
  message( FATAL_ERROR "${RDGENERAL_LIB}" )
endif()

find_library( INCHI_LIB NAMES RDKitInchi HINTS ${_libdir} )
find_library( INCHI_DEBUG_LIB NAMES RDKitInchi${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT INCHI_LIB )
  message( FATAL_ERROR "${INCHI_LIB}" )
endif()

find_library( RDINCHILIB_LIB NAMES RDKitRDInchiLib HINTS ${_libdir} )
find_library( RDINCHILIB_DEBUG_LIB NAMES RDKitRDInchiLib${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT RDINCHILIB_LIB )
  message( FATAL_ERROR "${RDINCHILIB_LIB}" )
endif()

find_library( FILTERCATALOG_LIB NAMES RDKitFilterCatalog HINTS ${_libdir} )
find_library( FILTERCATALOG_DEBUG_LIB NAMES RDKitFilterCatalog${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT FILTERCATALOG_LIB )
  message( FATAL_ERROR "${FILTERCATALOG_LIB}" )
endif()

find_library( CATALOGS_LIB NAMES RDKitCatalogs HINTS ${_libdir} )
find_library( CATALOGS_DEBUG_LIB NAMES RDKitCatalogs${CMAKE_DEBUG_POSTFIX} HINTS ${_libdir} )
if ( NOT CATALOGS_LIB )
  message( FATAL_ERROR "${CATALOGS_LIB}" )
endif()

if ( WIN32 )
  set_target_properties( RDKitChemReactions	PROPERTIES  IMPORTED_LOCATION ${CHEMREACTIONS_LIB}  IMPORTED_LOCATION_DEBUG ${CHEMREACTIONS_DEBUG_LIB} )
  set_target_properties( RDKitDataStructs	PROPERTIES  IMPORTED_LOCATION ${DATASTRUCTS_LIB}    IMPORTED_LOCATION_DEBUG ${DATASTRUCTS_DEBUG_LIB} )
  set_target_properties( RDKitDepictor		PROPERTIES  IMPORTED_LOCATION ${DEPICTOR_LIB}	    IMPORTED_LOCATION_DEBUG ${DEPICTOR_DEBUG_LIB} )
  set_target_properties( RDKitDescriptors	PROPERTIES  IMPORTED_LOCATION ${DESCRIPTORS_LIB}    IMPORTED_LOCATION_DEBUG ${DESCRIPTORS_DEBUG_LIB} )
  set_target_properties( RDKitEigenSolvers	PROPERTIES  IMPORTED_LOCATION ${EIGENSOLVERS_LIB}   IMPORTED_LOCATION_DEBUG ${EIGENSOLVERS_DEBUG_LIB} )
  set_target_properties( RDKitFileParsers	PROPERTIES  IMPORTED_LOCATION ${FILEPARSERS_LIB}    IMPORTED_LOCATION_DEBUG ${FILEPARSERS_DEBUG_LIB} )
  set_target_properties( RDKitFingerprints	PROPERTIES  IMPORTED_LOCATION ${FINGERPRINTS_LIB}   IMPORTED_LOCATION_DEBUG ${FINGERPRINTS_DEBUG_LIB} )
  set_target_properties( RDKitGraphMol		PROPERTIES  IMPORTED_LOCATION ${GRAPHMOL_LIB}	    IMPORTED_LOCATION_DEBUG ${GRAPHMOL_DEBUG_LIB} )
  set_target_properties( RDKitMolDraw2D		PROPERTIES  IMPORTED_LOCATION ${MOLDRAW2D_LIB}	    IMPORTED_LOCATION_DEBUG ${MOLDRAW2D_DEBUG_LIB} )
  set_target_properties( RDKitPartialCharges	PROPERTIES  IMPORTED_LOCATION ${PARTIALCHARGES_LIB} IMPORTED_LOCATION_DEBUG ${PARTIALCHARGES_DEBUG_LIB} )
  set_target_properties( RDKitSmilesParse	PROPERTIES  IMPORTED_LOCATION ${SMILESPARSE_LIB}    IMPORTED_LOCATION_DEBUG ${SMILESPARSE_DEBUG_LIB} )
  set_target_properties( RDKitSubstructMatch	PROPERTIES  IMPORTED_LOCATION ${SUBSTRUCTMATCH_LIB} IMPORTED_LOCATION_DEBUG ${SUBSTRUCTMATCH_DEBUG_LIB} )
  set_target_properties( RDKitChemTransforms	PROPERTIES  IMPORTED_LOCATION ${CHEMTRANSFORMS_LIB} IMPORTED_LOCATION_DEBUG ${CHEMTRANSFORMS_DEBUG_LIB} )
  set_target_properties( RDKitSubgraphs		PROPERTIES  IMPORTED_LOCATION ${SUBGRAPHS_LIB}	    IMPORTED_LOCATION_DEBUG ${SUBGRAPHS_DEBUG_LIB} )
  set_target_properties( RDKitMolTransforms	PROPERTIES  IMPORTED_LOCATION ${MOLTRANSFORMS_LIB}  IMPORTED_LOCATION_DEBUG ${MOLTRANSFORMS_DEBUG_LIB} )
  set_target_properties( RDKitRDGeometryLib	PROPERTIES  IMPORTED_LOCATION ${RDGEOMETRYLIB_LIB}  IMPORTED_LOCATION_DEBUG ${RDGEOMETRYLIB_DEBUG_LIB} )
  set_target_properties( RDKitRDGeneral		PROPERTIES  IMPORTED_LOCATION ${RDGENERAL_LIB}	    IMPORTED_LOCATION_DEBUG ${RDGENERAL_DEBUG_LIB} )
  set_target_properties( RDKitInchi		PROPERTIES  IMPORTED_LOCATION ${INCHI_LIB}	    IMPORTED_LOCATION_DEBUG ${INCHI_DEBUG_LIB} )
  set_target_properties( RDKitRDInchiLib	PROPERTIES  IMPORTED_LOCATION ${RDINCHILIB_LIB}	    IMPORTED_LOCATION_DEBUG ${RDINCHILIB_DEBUG_LIB} )
  set_target_properties( RDKitFilterCatalog	PROPERTIES  IMPORTED_LOCATION ${FILTERCATALOG_LIB}  IMPORTED_LOCATION_DEBUG ${FILTERCATALOG_DEBUG_LIB} )
  set_target_properties( RDKitCatalogs		PROPERTIES  IMPORTED_LOCATION ${CATALOGS_LIB}       IMPORTED_LOCATION_DEBUG ${CATALOGS_DEBUG_LIB} )
else()
  set_target_properties( RDKitChemReactions	PROPERTIES IMPORTED_LOCATION ${CHEMREACTIONS_LIB} )
  set_target_properties( RDKitDataStructs	PROPERTIES IMPORTED_LOCATION ${DATASTRUCTS_LIB} )
  set_target_properties( RDKitDepictor		PROPERTIES IMPORTED_LOCATION ${DEPICTOR_LIB} )
  set_target_properties( RDKitDescriptors	PROPERTIES IMPORTED_LOCATION ${DESCRIPTORS_LIB} )
  set_target_properties( RDKitEigenSolvers	PROPERTIES IMPORTED_LOCATION ${EIGENSOLVERS_LIB} )
  set_target_properties( RDKitFileParsers	PROPERTIES IMPORTED_LOCATION ${FILEPARSERS_LIB} )
  set_target_properties( RDKitFingerprints	PROPERTIES IMPORTED_LOCATION ${FINGERPRINTS_LIB} )
  set_target_properties( RDKitGraphMol		PROPERTIES IMPORTED_LOCATION ${GRAPHMOL_LIB} )
  set_target_properties( RDKitMolDraw2D		PROPERTIES IMPORTED_LOCATION ${MOLDRAW2D_LIB} )
  set_target_properties( RDKitPartialCharges	PROPERTIES IMPORTED_LOCATION ${PARTIALCHARGES_LIB} )
  set_target_properties( RDKitSmilesParse	PROPERTIES IMPORTED_LOCATION ${SMILESPARSE_LIB} )
  set_target_properties( RDKitSubstructMatch	PROPERTIES IMPORTED_LOCATION ${SUBSTRUCTMATCH_LIB} )
  set_target_properties( RDKitChemTransforms	PROPERTIES IMPORTED_LOCATION ${CHEMTRANSFORMS_LIB} )
  set_target_properties( RDKitSubgraphs		PROPERTIES IMPORTED_LOCATION ${SUBGRAPHS_LIB} )
  set_target_properties( RDKitMolTransforms	PROPERTIES IMPORTED_LOCATION ${MOLTRANSFORMS_LIB} )
  set_target_properties( RDKitRDGeometryLib	PROPERTIES IMPORTED_LOCATION ${RDGEOMETRYLIB_LIB} )
  set_target_properties( RDKitRDGeneral		PROPERTIES IMPORTED_LOCATION ${RDGENERAL_LIB} )
  set_target_properties( RDKitInchi		PROPERTIES IMPORTED_LOCATION ${INCHI_LIB} )
  set_target_properties( RDKitRDInchiLib	PROPERTIES IMPORTED_LOCATION ${RDINCHILIB_LIB} )
  set_target_properties( RDKitFilterCatalog	PROPERTIES IMPORTED_LOCATION ${FILTERCATALOG_LIB} )
  set_target_properties( RDKitCatalogs		PROPERTIES IMPORTED_LOCATION ${CATALOGS_LIB} )    
endif()

set ( RDKit_LIBRARIES
  RDKitCatalogs
  RDKitChemReactions
  RDKitDataStructs
  RDKitDepictor
  RDKitDescriptors  
  RDKitEigenSolvers
  RDKitFileParsers
  RDKitFilterCatalog
  RDKitFingerprints
  RDKitGraphMol
  RDKitMolDraw2D
  RDKitPartialCharges
  RDKitSmilesParse
  RDKitSubstructMatch
  RDKitChemTransforms
  RDKitSubgraphs
  RDKitMolTransforms
  RDKitRDGeometryLib
  RDKitRDGeneral
  RDKitInchi
  RDKitRDInchiLib
  )

set ( rdkit TRUE )
set ( rdkit_FOUND TRUE )
