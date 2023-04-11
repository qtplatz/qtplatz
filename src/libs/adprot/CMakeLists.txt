project( adprot )
cmake_policy(SET CMP0022 NEW)

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

target_link_libraries( adprot LINK_PUBLIC ${Boost_LIBRARIES} )

runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
  LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
  ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
  )
