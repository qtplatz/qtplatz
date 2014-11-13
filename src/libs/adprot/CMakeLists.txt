
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

