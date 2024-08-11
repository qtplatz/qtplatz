cmake_minimum_required(VERSION 3.30)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )
set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )

project( pfoa_rxn )

set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_INCLUDE_CURRENT_DIR ON )

if ( NOT rdkit_FOUND )
  set ( VERBOSE ON )
  find_package( Eigen3 REQUIRED )
  find_package( rdkit QUIET )
endif()
message( STATUS "rdkit_FOUND: ${rdkit_FOUND}" )
if ( rdkit_FOUND )
  message( STATUS "RDKit_INCLUDE_DIRS: ${RDKit_INCLUDE_DIRS}" )
  message( STATUS "RDKit_LIBRARIES: ${RDKit_LIBRARIES}" )

  get_filename_component( _dir "${rdkit_CONFIG}" PATH )
  find_file( version_cmake NAMES "rdkit-config-version.cmake" PATHS ${_dir} NO_DEFAULT_PATH )
  if ( version_cmake )
    include( ${version_cmake} )
    set( RDKit_PACKAGE_VERSION ${PACKAGE_VERSION} )
    message( STATUS "RDKit_PACKAGE_VERSION: ${PACKAGE_VERSION}" )
  endif()

  add_definitions( -DHAVE_RDKit=1 )
endif()

include_directories(
  "${Boost_INCLUDE_DIRS}"
  "${RDKit_INCLUDE_DIRS}"
  "../../src/libs"  # adportable
)

add_executable( ${PROJECT_NAME}
  main.cpp
  ../../src/libs/adportable/smarts_parser.hpp
  ../../src/libs/adportable/smarts_parser.cpp
  allchem.cpp allchem.hpp
  printer.cpp printer.hpp
  drawer.cpp drawer.hpp
  html.cpp html.hpp
)

add_executable( example17
  example17.cpp
)

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  "${Boost_LIBRARIES}"
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
  RDKit::RDStreams
)

target_link_libraries( example17 LINK_PUBLIC
  "${Boost_LIBRARIES}"
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
  RDKit::RDStreams
)

if ( Boost_FOUND )
  message( STATUS "\# Boost_LIBRARIES" )
  foreach( _lib ${Boost_LIBRARIES} )
    if ( NOT ( ${_lib} STREQUAL "debug" OR ${_lib} STREQUAL "optimized" ) )
      message( STATUS "\t${_lib}" )
    endif()
  endforeach()
endif()
