
#message( STATUS "##### rdkit-cpack.cmake<qtplatz> #####" )

include( "soname" )

if ( rdkit_FOUND )

  foreach( lib
      DataStructs
      Depictor
      Descriptors       
      FileParsers
      GraphMol
      PartialCharges
      RDGeneral
      RDGeometryLib
      SmilesParse
      Subgraphs
      SubstructMatch )

    get_target_property( _loc ${lib} LOCATION )
    #message( STATUS "## rdkit-cpack install " ${_loc} " --> runtime_libraries" )
  
    if ( WIN32 )
      install( FILES ${_loc} DESTINATION ${dest} COMPONENT runtime_libraries )
    else()
      get_filename_component( name ${_loc} NAME_WE )
      get_filename_component( path ${_loc} DIRECTORY )
      file( GLOB files "${path}/${name}.${SO}.*" )
      install( PROGRAMS ${files} DESTINATION ${dest} COMPONENT runtime_libraries )
    endif()
  
  endforeach()

endif()

