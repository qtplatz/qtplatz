
add_definitions(-DADPROT_LIBRARY)

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

