# adchem
project( adchem )

cmake_policy( SET CMP0022 NEW )

add_definitions( -DADCHEM_LIBRARY )

find_package( Boost REQUIRED COMPONENTS system )

include_directories( ${RDKit_INCLUDE_DIRS} )
link_directories( ${rdkit_DIR} )

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

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
