cmake_minimum_required( VERSION 3.5 )
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)
cmake_policy(SET CMP0028 NEW)

project( xyz )

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#  find_package( BZip2 REQUIRED )

include_directories(
  ${CMAKE_SOURCE_DIR}/src
  ${RDKit_INCLUDE_DIRS}
  )

add_executable( ${PROJECT_NAME}
  main.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adportable
  adchem
  ${Boost_LIBRARIES}
  RDKit::Catalogs
  RDKit::ChemReactions
  RDKit::ChemTransforms
  RDKit::DataStructs
  RDKit::Depictor
  RDKit::Descriptors
  RDKit::DistGeomHelpers
  RDKit::DistGeometry
  RDKit::EigenSolvers
  RDKit::FileParsers
  RDKit::FilterCatalog
  RDKit::Fingerprints
  RDKit::ForceField
  RDKit::ForceFieldHelpers
  RDKit::GraphMol
  RDKit::Inchi
  RDKit::MolAlign
  RDKit::MolDraw2D
  RDKit::MolTransforms
  RDKit::Optimizer
  RDKit::PartialCharges
  RDKit::RDGeneral
  RDKit::RDGeometryLib
  RDKit::RDInchiLib
  RDKit::RDStreams
  RDKit::SmilesParse
  RDKit::Subgraphs
  RDKit::SubstructMatch
)

if ( APPLE )

  add_executable( molvolume
    molvolume.cpp
  )

  target_link_libraries( molvolume LINK_PUBLIC
    adportable
    adchem
    ${Boost_LIBRARIES}
    RDKit::Catalogs
    RDKit::ChemReactions
    RDKit::ChemTransforms
    RDKit::DataStructs
    RDKit::Depictor
    RDKit::Descriptors
    RDKit::DistGeomHelpers
    RDKit::DistGeometry
    RDKit::EigenSolvers
    RDKit::FileParsers
    RDKit::FilterCatalog
    RDKit::Fingerprints
    RDKit::ForceField
    RDKit::ForceFieldHelpers
    RDKit::GraphMol
    RDKit::Inchi
    RDKit::MolAlign
    RDKit::MolDraw2D
    RDKit::MolTransforms
    RDKit::Optimizer
    RDKit::PartialCharges
    RDKit::RDGeneral
    RDKit::RDGeometryLib
    RDKit::RDInchiLib
    RDKit::RDStreams
    RDKit::SmilesParse
    RDKit::Subgraphs
    RDKit::SubstructMatch
    RDKit::ShapeHelpers   # <---
  )
endif()

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
)

if ( APPLE )
  install( TARGETS molvolume
    RUNTIME DESTINATION bin COMPONENT applications
    LIBRARY DESTINATION lib/qtplatz COMPONENT libraries
    ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )
endif()
