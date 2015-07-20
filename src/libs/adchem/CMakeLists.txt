# adchem
project( adchem )

cmake_policy( SET CMP0022 NEW )

add_definitions( -DADCHEM_LIBRARY )
if ( MSVC )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996" )
endif()

find_package( Boost REQUIRED COMPONENTS system )
find_package( rdkit )

include_directories( ${RDKit_INCLUDE_DIRS} )

add_library( adchem SHARED
  adchem.cpp
  sdfile.cpp
  molfile.cpp
  drawing.cpp
  molecule.cpp
  )

target_link_libraries( adchem LINK_PUBLIC
  adportable
  FileParsers
  SmilesParse
  Depictor
  Descriptors
  GraphMol
  RDGeometryLib
  RDGeneral
  SubstructMatch
  ${Boost_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
