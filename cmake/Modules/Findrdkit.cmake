# find RDKit

if ( rdkit_FOUND )
  return()
endif()

set ( rdkit "rdkit-NOTFOUND" )
set ( _rdkit_libdirs "${RDBASE}/lib" "$ENV{RDBASE}/lib" "${CMAKE_SOURCE_DIR}/../rdkit/lib" )

if ( WIN32 )
  list ( APPEND _rdkit_libdirs "$ENV{PROGRAMFILES}/RDKit/lib" )
endif()

find_file( rdkit_config_cmake "rdkit-config.cmake" PATHS ${_rdkit_libdirs} )

if ( rdkit_config_cmake )

  # message( STATUS "###### rdkit-config.cmake : " ${rdkit_config_cmake} )

  include( ${rdkit_config_cmake} )

  get_filename_component( _dir "${rdkit_config_cmake}" PATH )
  get_filename_component( _prefix "${_dir}/.." ABSOLUTE )

  set ( RDKit_LIBRARY_DIRS ${_dir} )

  find_file( version_cmake NAMES "rdkit-config-version.cmake" PATHS ${_dir} NO_DEFAULT_PATH )
  if ( version_cmake )
    include( ${version_cmake} )
    set( RDKit_PACKAGE_VERSION ${PACKAGE_VERSION} )
  endif()

  find_path( _inchi_include "INCHI-API/inchi.h" PATHS "${_prefix}/Code" "${_prefix}/External" )
  if ( _inchi_include )
    list( APPEND RDKit_INCLUDE_DIRS ${_inchi_include} )
  endif()

  find_path( _moldraw2d_include "MolDraw2DSVG.h" PATHS "${_prefix/GraphMol/MolDraw2D}" )
  if ( _moldraw2d_include )
    list( APPEND RDKit_INCLUDE_DIRS ${_moldraw2d_include} )
  endif()
  
  set ( RDKit_LIBRARIES
    Catalogs
    ChemReactions
    DataStructs
    Depictor
    Descriptors  
    EigenSolvers
    FileParsers
    FilterCatalog
    Fingerprints
    GraphMol
    MolDraw2D
    PartialCharges
    SubstructMatch
    ChemTransforms
    Subgraphs
    MolTransforms
    RDGeometryLib
    RDGeneral
    Inchi
    RDInchiLib
    SmilesParse
    )
  set( rdkit_FOUND TRUE )

  return()
endif()

message( STATUS "###### rdkit-config.cmake NOT FOUND -- Continue local lookup #####" )

set( _rdkit_incdirs
  "${RDBASE}/Code"  
  "${RDBASE}/External"
  "$ENV{RDBASE}/Code"  
  "$ENV{RDBASE}/External"
  "${CMAKE_SOURCE_DIR}/../rdkit/Code"
  "${CMAKE_SOURCE_DIR}/../rdkit/External"
  )

find_path( _include_dir GraphMol/RDKitBase.h HINTS ${_rdkit_incdirs} )

if ( NOT _include_dir )
  return()
endif()

find_path( _inchi_inc_dir INCHI-API/inchi.h HINTS ${_rdkit_incdirs} )

find_library( _fileparsers_lib NAMES "RDKitFileParsers" HINTS
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

message( STATUS "## Findrdkit.cmake ## RDBASE = " ${RDBASE} )

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

#[[
if ( WIN32 )
  add_library( RDKit::ChemReactions	STATIC IMPORTED )
  add_library( RDKit::DataStructs	STATIC IMPORTED )
  add_library( RDKit::Depictor		STATIC IMPORTED )
  add_library( RDKit::Descriptors	STATIC IMPORTED )
  add_library( RDKit::EigenSolvers	STATIC IMPORTED )
  add_library( RDKit::FileParsers	STATIC IMPORTED )
  add_library( RDKit::Fingerprints	STATIC IMPORTED )
  add_library( RDKit::GraphMol		STATIC IMPORTED )
  add_library( RDKit::MolDraw2D		STATIC IMPORTED )
  add_library( RDKit::PartialCharges	STATIC IMPORTED )
  add_library( RDKit::SmilesParse	STATIC IMPORTED )
  add_library( RDKit::SubstructMatch	STATIC IMPORTED )
  add_library( RDKit::ChemTransforms	STATIC IMPORTED )
  add_library( RDKit::Subgraphs		STATIC IMPORTED )
  add_library( RDKit::MolTransforms	STATIC IMPORTED )
  add_library( RDKit::RDGeometryLib	STATIC IMPORTED )
  add_library( RDKit::RDGeneral		STATIC IMPORTED )
  add_library( RDKit::Inchi		STATIC IMPORTED )
  add_library( RDKit::RDInchiLib	STATIC IMPORTED )
  add_library( RDKit::FilterCatalog	STATIC IMPORTED )
  add_library( RDKit::Catalogs		STATIC IMPORTED )
else()
  ]]
  add_library( RDKit::ChemReactions	SHARED IMPORTED )
  add_library( RDKit::DataStructs	SHARED IMPORTED )
  add_library( RDKit::Depictor		SHARED IMPORTED )
  add_library( RDKit::Descriptors	SHARED IMPORTED )
  add_library( RDKit::EigenSolvers	SHARED IMPORTED )
  add_library( RDKit::FileParsers	SHARED IMPORTED )
  add_library( RDKit::Fingerprints	SHARED IMPORTED )
  add_library( RDKit::GraphMol		SHARED IMPORTED )
  add_library( RDKit::MolDraw2D		SHARED IMPORTED )
  add_library( RDKit::PartialCharges	SHARED IMPORTED )
  add_library( RDKit::SmilesParse	SHARED IMPORTED )
  add_library( RDKit::SubstructMatch	SHARED IMPORTED )
  add_library( RDKit::ChemTransforms	SHARED IMPORTED )
  add_library( RDKit::Subgraphs		SHARED IMPORTED )
  add_library( RDKit::MolTransforms	SHARED IMPORTED )
  add_library( RDKit::RDGeometryLib	SHARED IMPORTED )
  add_library( RDKit::RDGeneral		SHARED IMPORTED )
  add_library( RDKit::Inchi		SHARED IMPORTED )
  add_library( RDKit::RDInchiLib	SHARED IMPORTED )
  add_library( RDKit::FilterCatalog	SHARED IMPORTED )
  add_library( RDKit::Catalogs		SHARED IMPORTED )
#endif()

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
  set_target_properties( RDKit::ChemReactions	PROPERTIES  IMPORTED_LOCATION_RELEASE ${CHEMREACTIONS_LIB}  IMPORTED_LOCATION_DEBUG ${CHEMREACTIONS_DEBUG_LIB} )
  set_target_properties( RDKit::DataStructs	PROPERTIES  IMPORTED_LOCATION_RELEASE ${DATASTRUCTS_LIB}    IMPORTED_LOCATION_DEBUG ${DATASTRUCTS_DEBUG_LIB} )
  set_target_properties( RDKit::Depictor	PROPERTIES  IMPORTED_LOCATION_RELEASE ${DEPICTOR_LIB}	    IMPORTED_LOCATION_DEBUG ${DEPICTOR_DEBUG_LIB} )
  set_target_properties( RDKit::Descriptors	PROPERTIES  IMPORTED_LOCATION_RELEASE ${DESCRIPTORS_LIB}    IMPORTED_LOCATION_DEBUG ${DESCRIPTORS_DEBUG_LIB} )
  set_target_properties( RDKit::EigenSolvers	PROPERTIES  IMPORTED_LOCATION_RELEASE ${EIGENSOLVERS_LIB}   IMPORTED_LOCATION_DEBUG ${EIGENSOLVERS_DEBUG_LIB} )
  set_target_properties( RDKit::FileParsers	PROPERTIES  IMPORTED_LOCATION_RELEASE ${FILEPARSERS_LIB}    IMPORTED_LOCATION_DEBUG ${FILEPARSERS_DEBUG_LIB} )
  set_target_properties( RDKit::Fingerprints	PROPERTIES  IMPORTED_LOCATION_RELEASE ${FINGERPRINTS_LIB}   IMPORTED_LOCATION_DEBUG ${FINGERPRINTS_DEBUG_LIB} )
  set_target_properties( RDKit::GraphMol	PROPERTIES  IMPORTED_LOCATION_RELEASE ${GRAPHMOL_LIB}	    IMPORTED_LOCATION_DEBUG ${GRAPHMOL_DEBUG_LIB} )
  set_target_properties( RDKit::MolDraw2D	PROPERTIES  IMPORTED_LOCATION_RELEASE ${MOLDRAW2D_LIB}	    IMPORTED_LOCATION_DEBUG ${MOLDRAW2D_DEBUG_LIB} )
  set_target_properties( RDKit::PartialCharges	PROPERTIES  IMPORTED_LOCATION_RELEASE ${PARTIALCHARGES_LIB} IMPORTED_LOCATION_DEBUG ${PARTIALCHARGES_DEBUG_LIB} )
  set_target_properties( RDKit::SmilesParse	PROPERTIES  IMPORTED_LOCATION_RELEASE ${SMILESPARSE_LIB}    IMPORTED_LOCATION_DEBUG ${SMILESPARSE_DEBUG_LIB} )
  set_target_properties( RDKit::SubstructMatch	PROPERTIES  IMPORTED_LOCATION_RELEASE ${SUBSTRUCTMATCH_LIB} IMPORTED_LOCATION_DEBUG ${SUBSTRUCTMATCH_DEBUG_LIB} )
  set_target_properties( RDKit::ChemTransforms	PROPERTIES  IMPORTED_LOCATION_RELEASE ${CHEMTRANSFORMS_LIB} IMPORTED_LOCATION_DEBUG ${CHEMTRANSFORMS_DEBUG_LIB} )
  set_target_properties( RDKit::Subgraphs	PROPERTIES  IMPORTED_LOCATION_RELEASE ${SUBGRAPHS_LIB}	    IMPORTED_LOCATION_DEBUG ${SUBGRAPHS_DEBUG_LIB} )
  set_target_properties( RDKit::MolTransforms	PROPERTIES  IMPORTED_LOCATION_RELEASE ${MOLTRANSFORMS_LIB}  IMPORTED_LOCATION_DEBUG ${MOLTRANSFORMS_DEBUG_LIB} )
  set_target_properties( RDKit::RDGeometryLib	PROPERTIES  IMPORTED_LOCATION_RELEASE ${RDGEOMETRYLIB_LIB}  IMPORTED_LOCATION_DEBUG ${RDGEOMETRYLIB_DEBUG_LIB} )
  set_target_properties( RDKit::RDGeneral	PROPERTIES  IMPORTED_LOCATION_RELEASE ${RDGENERAL_LIB}	    IMPORTED_LOCATION_DEBUG ${RDGENERAL_DEBUG_LIB} )
  set_target_properties( RDKit::Inchi		PROPERTIES  IMPORTED_LOCATION_RELEASE ${INCHI_LIB}	    IMPORTED_LOCATION_DEBUG ${INCHI_DEBUG_LIB} )
  set_target_properties( RDKit::RDInchiLib	PROPERTIES  IMPORTED_LOCATION_RELEASE ${RDINCHILIB_LIB}	    IMPORTED_LOCATION_DEBUG ${RDINCHILIB_DEBUG_LIB} )
  set_target_properties( RDKit::FilterCatalog	PROPERTIES  IMPORTED_LOCATION_RELEASE ${FILTERCATALOG_LIB}  IMPORTED_LOCATION_DEBUG ${FILTERCATALOG_DEBUG_LIB} )
  set_target_properties( RDKit::Catalogs	PROPERTIES  IMPORTED_LOCATION_RELEASE ${CATALOGS_LIB}       IMPORTED_LOCATION_DEBUG ${CATALOGS_DEBUG_LIB} )
else()
  set_target_properties( RDKit::ChemReactions	PROPERTIES IMPORTED_LOCATION_RELEASE ${CHEMREACTIONS_LIB} )
  set_target_properties( RDKit::DataStructs	PROPERTIES IMPORTED_LOCATION_RELEASE ${DATASTRUCTS_LIB} )
  set_target_properties( RDKit::Depictor	PROPERTIES IMPORTED_LOCATION_RELEASE ${DEPICTOR_LIB} )
  set_target_properties( RDKit::Descriptors	PROPERTIES IMPORTED_LOCATION_RELEASE ${DESCRIPTORS_LIB} )
  set_target_properties( RDKit::EigenSolvers	PROPERTIES IMPORTED_LOCATION_RELEASE ${EIGENSOLVERS_LIB} )
  set_target_properties( RDKit::FileParsers	PROPERTIES IMPORTED_LOCATION_RELEASE ${FILEPARSERS_LIB} )
  set_target_properties( RDKit::Fingerprints	PROPERTIES IMPORTED_LOCATION_RELEASE ${FINGERPRINTS_LIB} )
  set_target_properties( RDKit::GraphMol	PROPERTIES IMPORTED_LOCATION_RELEASE ${GRAPHMOL_LIB} )
  set_target_properties( RDKit::MolDraw2D	PROPERTIES IMPORTED_LOCATION_RELEASE ${MOLDRAW2D_LIB} )
  set_target_properties( RDKit::PartialCharges	PROPERTIES IMPORTED_LOCATION_RELEASE ${PARTIALCHARGES_LIB} )
  set_target_properties( RDKit::SmilesParse	PROPERTIES IMPORTED_LOCATION_RELEASE ${SMILESPARSE_LIB} )
  set_target_properties( RDKit::SubstructMatch	PROPERTIES IMPORTED_LOCATION_RELEASE ${SUBSTRUCTMATCH_LIB} )
  set_target_properties( RDKit::ChemTransforms	PROPERTIES IMPORTED_LOCATION_RELEASE ${CHEMTRANSFORMS_LIB} )
  set_target_properties( RDKit::Subgraphs	PROPERTIES IMPORTED_LOCATION_RELEASE ${SUBGRAPHS_LIB} )
  set_target_properties( RDKit::MolTransforms	PROPERTIES IMPORTED_LOCATION_RELEASE ${MOLTRANSFORMS_LIB} )
  set_target_properties( RDKit::RDGeometryLib	PROPERTIES IMPORTED_LOCATION_RELEASE ${RDGEOMETRYLIB_LIB} )
  set_target_properties( RDKit::RDGeneral	PROPERTIES IMPORTED_LOCATION_RELEASE ${RDGENERAL_LIB} )
  set_target_properties( RDKit::Inchi		PROPERTIES IMPORTED_LOCATION_RELEASE ${INCHI_LIB} )
  set_target_properties( RDKit::RDInchiLib	PROPERTIES IMPORTED_LOCATION_RELEASE ${RDINCHILIB_LIB} )
  set_target_properties( RDKit::FilterCatalog	PROPERTIES IMPORTED_LOCATION_RELEASE ${FILTERCATALOG_LIB} )
  set_target_properties( RDKit::Catalogs	PROPERTIES IMPORTED_LOCATION_RELEASE ${CATALOGS_LIB} )    
endif()

set ( RDKit_LIBRARIES
  RDKit::Catalogs
  RDKit::ChemReactions
  RDKit::DataStructs
  RDKit::Depictor
  RDKit::Descriptors  
  RDKit::EigenSolvers
  RDKit::FileParsers
  RDKit::FilterCatalog
  RDKit::Fingerprints
  RDKit::GraphMol
  RDKit::MolDraw2D
  RDKit::PartialCharges
  RDKit::SubstructMatch
  RDKit::ChemTransforms
  RDKit::Subgraphs
  RDKit::MolTransforms
  RDKit::RDGeometryLib
  RDKit::RDGeneral
  RDKit::Inchi
  RDKit::RDInchiLib
  RDKit::SmilesParse
  )

set ( rdkit TRUE )
set ( rdkit_FOUND TRUE )
