project( adprot )
cmake_policy(SET CMP0022 NEW)

add_definitions(-DADPROT_LIBRARY)

find_package(Boost COMPONENTS system filesystem )

add_library( adprot SHARED
  adprot.cpp 
  peptide.cpp 
  peptides.cpp 
  protease.cpp 
  protein.cpp 
  sequence.cpp 
  protfile.cpp 
  aminoacid.cpp 
  digestedpeptides.cpp
)

target_link_libraries( adprot LINK_PUBLIC ${Boost_LIBRARIES} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules    
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
