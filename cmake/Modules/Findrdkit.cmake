# find RDKit

if ( rdkit_FOUND )
  return()
endif()

set ( rdkit "rdkit-NOTFOUND" )
set ( _rdkit_libdirs "$ENV{PROGRAMFILES}/RDKit/lib/cmake/rdkit" )

if ( WIN32 )
  list ( APPEND _rdkit_libdirs
    "C:/opt/RDKit/lib/cmake/rdkit" )
else()
  list ( APPEND _rdkit_libdirs
    "/usr/local/lib/cmake/rdkit" )
endif()

find_package( rdkit CONFIG HINTS ${_rdkit_libdirs} )

###### for compatibility with older scripts ###########

get_filename_component( _dir "${rdkit_CONFIG}" PATH )
find_file( version_cmake NAMES "rdkit-config-version.cmake" PATHS ${_dir} NO_DEFAULT_PATH )

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

if ( _path )

  get_filename_component( RDKit_LIBRARY_DIRS ${_path} PATH )

  if ( rdkit_import_prefix )

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
endif()
