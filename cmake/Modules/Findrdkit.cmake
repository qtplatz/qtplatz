# find RDKit

if ( rdkit_FOUND )
  return()
endif()

set ( rdkit "rdkit-NOTFOUND" )

if ( DEFINED $ENV{RDBASE} )
  set ( _rdkit_libdirs "$ENV{RDBASE}/lib" )
endif()
if ( WIN32 )
  list ( APPEND _rdkit_libdirs "C:/RDKit/lib/cmake/rdkit" "$ENV{PROGRAMFILES}/RDKit/lib/cmake/rdkit" )
else()
  list ( APPEND _rdkit_libdirs "/usr/local/lib/cmake/rdkit" )
endif()
list ( APPEND _rdkit_libdirs ${RDBASE} "${CMAKE_SOURCE_DIR}/../rdkit/lib/cmake/rdkit" )

find_file( rdkit_config_cmake "rdkit-config.cmake" PATHS ${_rdkit_libdirs} )

if ( rdkit_config_cmake )
  message( STATUS "##### found rdkit_config_cmake: " ${rdkit_config_cmake} )

  include( ${rdkit_config_cmake} )

  get_filename_component( _dir "${rdkit_config_cmake}" PATH )
  get_filename_component( _prefix "${_dir}/.." ABSOLUTE )

  find_file( version_cmake NAMES "rdkit-config-version.cmake" PATHS ${_dir} NO_DEFAULT_PATH )

  if ( VERBOSE )
    message( STATUS "##### found rdkit_config_cmake dir: " ${_dir} )
    message( STATUS "##### found rdkit_config_cmake prefix: " ${_prefix} )
    message( STATUS "##### version_cmake: " ${version_cmake} )
  endif()

  if ( version_cmake )
    include( ${version_cmake} )
    set( RDKit_PACKAGE_VERSION ${PACKAGE_VERSION} )
  endif()

  if ( VERBOSE )
    message( STATUS "##### find rdkit.cmake -- RDKit_PACKAGE_VERSION = " ${RDKit_PACKAGE_VERSION} )
  endif()

  if ( TARGET RDGeneral )
    set ( rdkit_import_prefix FALSE )
    get_target_property( _path RDGeneral IMPORTED_LOCATION_RELEASE )
  else()
    set ( rdkit_import_prefix TRUE )
    get_target_property( _path RDKit::RDGeneral IMPORTED_LOCATION_RELEASE )
  endif()

  if ( NOT _path )
    message( FATAL_ERROR "##### Cannot get RDKit library location #####" )
    return()
  endif()

  get_filename_component( RDKit_LIBRARY_DIRS ${_path} PATH )

  if ( rdkit_import_prefix )
#[[
foreach(tgt
    RDKit::Abbreviations
    RDKit::Alignment
    RDKit::CIPLabeler
    RDKit::Catalogs
    RDKit::ChemReactions
    RDKit::ChemTransforms
    RDKit::ChemicalFeatures
    RDKit::DataStructs
    RDKit::Depictor
    RDKit::Deprotect
    RDKit::Descriptors
    RDKit::DistGeomHelpers
    RDKit::DistGeometry
    RDKit::EigenSolvers
    RDKit::FMCS
    RDKit::FileParsers
    RDKit::FilterCatalog
    RDKit::Fingerprints
    RDKit::ForceField
    RDKit::ForceFieldHelpers
    RDKit::FragCatalog
    RDKit::GraphMol
    RDKit::Inchi
    RDKit::InfoTheory
    RDKit::MMPA
    RDKit::MolAlign
    RDKit::MolCatalog
    RDKit::MolChemicalFeatures
    RDKit::MolDraw2D
    RDKit::MolEnumerator
    RDKit::MolHash
    RDKit::MolInterchange
    RDKit::MolStandardize
    RDKit::MolTransforms
    RDKit::O3AAlign
    RDKit::Optimizer
    RDKit::PartialCharges
    RDKit::RDGeneral
    RDKit::RDGeometryLib
    RDKit::RDInchiLib
    RDKit::RDStreams
    RDKit::RGroupDecomposition
    RDKit::ReducedGraphs
    RDKit::RingDecomposerLib
    RDKit::SLNParse
    RDKit::ScaffoldNetwork
    RDKit::ShapeHelpers
    RDKit::SimDivPickers
    RDKit::SmilesParse
    RDKit::Subgraphs
    RDKit::SubstructLibrary
    RDKit::SubstructMatch
    RDKit::TautomerQuery
    RDKit::Trajectory
    RDKit::coordgen
    RDKit::hc
    RDKit::maeparser
    )
      get_target_property( path ${tgt} IMPORTED_LOCATION_RELEASE )
      if ( path )
        list( APPEND RDKit_LIBRARIES ${tgt} )
      endif()
    endforeach()
]]
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
  else()
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
  endif()
  set( rdkit_FOUND TRUE )
  return()
endif()

message( STATUS "###### rdkit-config.cmake NOT FOUND -- Continue local lookup #####" )
