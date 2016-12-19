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

if ( WIN32 )
  set_target_properties( ChemReactions	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/ChemReactions.lib	IMPORTED_IMPLIB_DEBUG ${_libdir}/ChemReactionsd.lib )
  set_target_properties( DataStructs	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/DataStructs.lib		IMPORTED_IMPLIB_DEBUG ${_libdir}/DataStructsd.lib )
  set_target_properties( Depictor	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/Depictor.lib		IMPORTED_IMPLIB_DEBUG ${_libdir}/Depictord.lib )
  set_target_properties( Descriptors	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/Descriptors.lib		IMPORTED_IMPLIB_DEBUG ${_libdir}/Descriptorsd.lib )
  set_target_properties( EigenSolvers	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/EigenSolvers.lib		IMPORTED_IMPLIB_DEBUG ${_libdir}/EigenSolversd.lib )
  set_target_properties( FileParsers	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/FileParsers.lib		IMPORTED_IMPLIB_DEBUG ${_libdir}/FileParsersd.lib )
  set_target_properties( Fingerprints	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/Fingerprints.lib		IMPORTED_IMPLIB_DEBUG ${_libdir}/Fingerprintsd.lib )
  set_target_properties( GraphMol	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/GraphMol.lib		IMPORTED_IMPLIB_DEBUG ${_libdir}/GraphMold.lib )
  set_target_properties( MolDraw2D	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/MolDraw2D.lib		IMPORTED_IMPLIB_DEBUG ${_libdir}/MolDraw2Dd.lib )
  set_target_properties( PartialCharges	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/PartialCharges.lib	IMPORTED_IMPLIB_DEBUG ${_libdir}/PartialChargesd.lib )
  set_target_properties( SmilesParse	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/SmilesParse.lib		IMPORTED_IMPLIB_DEBUG ${_libdir}/SmilesParsed.lib )
  set_target_properties( SubstructMatch	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/SubstructMatch.lib	IMPORTED_IMPLIB_DEBUG ${_libdir}/SubstructMatchd.lib )
  set_target_properties( ChemTransforms	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/ChemTransforms.lib	IMPORTED_IMPLIB_DEBUG ${_libdir}/ChemTransformsd.lib )
  set_target_properties( Subgraphs	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/Subgraphs.lib		IMPORTED_IMPLIB_DEBUG ${_libdir}/Subgraphsd.lib )
  set_target_properties( MolTransforms	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/MolTransforms.lib	IMPORTED_IMPLIB_DEBUG ${_libdir}/MolTransformsd.lib )
  set_target_properties( RDGeometryLib	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/RDGeometryLib.lib	IMPORTED_IMPLIB_DEBUG ${_libdir}/RDGeometryLibd.lib )
  set_target_properties( RDGeneral	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/RDGeneral.lib		IMPORTED_IMPLIB_DEBUG ${_libdir}/RDGenerald.lib )
  set_target_properties( Inchi		PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/Inchi.lib		IMPORTED_IMPLIB_DEBUG ${_libdir}/Inchid.lib )
  set_target_properties( RDInchiLib	PROPERTIES
    IMPORTED_IMPLIB ${_libdir}/RDInchiLib.lib		IMPORTED_IMPLIB_DEBUG ${_libdir}/RDInchiLibd.lib )  
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

