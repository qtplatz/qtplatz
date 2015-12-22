# adchem
cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0022 NEW )

project( adchem )

find_package( Boost REQUIRED COMPONENTS system )
find_package( rdkit )

add_definitions( -DADCHEM_LIBRARY )

if ( MSVC )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996" )
endif()

include_directories( ${RDKit_INCLUDE_DIRS} )

set( adchem_SOURCES
  adchem.cpp
  sdfile.cpp
  molfile.cpp
  drawing.cpp
  molecule.cpp
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
