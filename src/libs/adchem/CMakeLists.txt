# adchem
cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0022 NEW )

project( adchem )

if ( NOT Boost_FOUND OR WIN32 )
  find_package( Boost REQUIRED COMPONENTS system thread )
endif()

find_package( rdkit )

add_definitions( -DADCHEM_LIBRARY )

include_directories( ${RDKit_INCLUDE_DIRS} )

set( adchem_SOURCES
  adchem.cpp
  adchem.hpp
  sdfile.cpp
  sdfile.hpp
  mol.cpp
  mol.hpp
  drawing.cpp
  drawing.hpp
  )

if ( WIN32 )
  add_library( adchem SHARED ${adchem_SOURCES} )
  add_definitions( -wd4996 -wd4348 )
else()
  # workaround -- I couldn't fix an issue 'shared lib image not find' on mac even play with install_name change.
  add_library( adchem ${adchem_SOURCES} )  
endif()

target_link_libraries( adchem LINK_PUBLIC
  adportable
  ${RDKit_LIBRARIES}
  ${Boost_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
