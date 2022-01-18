# adchem
cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0022 NEW )

project( adchem )

add_definitions( -DADCHEM_LIBRARY )
find_package(TBB REQUIRED)

include_directories( ${RDKit_INCLUDE_DIRS} )

if ( rdkit_FOUND )
  set( rdkit_SOURCES
    sdfile.cpp
    sdfile.hpp
    mol.cpp
    mol.hpp
    drawing.cpp
    drawing.hpp
    )
endif()

add_library( adchem SHARED
  sdfile_parser.hpp
  sdmolsupplier.cpp
  sdmolsupplier.hpp
  smilestosvg.cpp
  smilestosvg.hpp
  adchem.cpp
  adchem.hpp
  ${rdkit_SOURCES}
  )

target_link_libraries( adchem LINK_PUBLIC
  adportable
  TBB::tbb
  ${Boost_LIBRARIES}
  ${RDKit_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
