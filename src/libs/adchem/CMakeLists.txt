# adchem
cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0022 NEW )

project( adchem )

#find_package( rdkit )
if ( NOT rdkit_FOUND )
  return()
endif()

add_definitions( -DADCHEM_LIBRARY )

include_directories( ${RDKit_INCLUDE_DIRS} )
#[[
foreach( a ${RDKit_LIBRARIES} )
  message( STATUS "-----------> " ${a} )
endforeach()
]]

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

add_library( adchem SHARED ${adchem_SOURCES} )

target_link_libraries( adchem LINK_PUBLIC
  adportable
  ${Boost_LIBRARIES}
  ${RDKit_LIBRARIES}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

