# adchem
cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0022 NEW )

project( adchem )

add_definitions( -DADCHEM_LIBRARY )
find_package(TBB QUIET)

if ( TBB_FOUND )
  add_definitions( -DHAVE_TBB=1 )
  set ( TBB_LIBRARIES "TBB::tbb" )
endif()

include_directories( ${RDKit_INCLUDE_DIRS} )

if ( rdkit_FOUND )
  set( rdkit_SOURCES
    sdfile.cpp
    sdfile.hpp
    sdmol.cpp
    sdmol.hpp
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
  ${TBB_LIBRARIES}
  ${Boost_LIBRARIES}
  ${RDKit_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib COMPONENT libraries )

install( FILES
  adchem_global.hpp
  sdfile.hpp
  sdmol.hpp
  mol.hpp
  drawing.hpp
  sdmolsupplier.hpp
  smilestosvg.hpp
  adchem.hpp
  DESTINATION include/adchem COMPONENT headers )
