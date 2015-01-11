# adchem

include( ${RDKIT_CONFIG_DIR}/rdkit-config.cmake )

add_definitions( -DADCHEM_LIBRARY )
include_directories( ${RDKit_INCLUDE_DIRS} )

add_library( adchem SHARED
  adchem.cpp
  sdfile.cpp
  molfile.cpp
  drawing.cpp
  molecule.cpp
)

target_link_libraries( adchem LINK_PUBLIC 
  FileParsers
  GraphMol
  SmilesParse
  RDGeneral
  RDGeometryLib
  SubstructMatch
  Depictor
  Descriptors
  adportable
  ${Boost_LIBRARIES} )
