
cmake_policy( SET CMP0045 NEW )

include( "soname" )

if ( rdkit_FOUND )

  file( GLOB _libs "${RDKit_LIBRARY_DIRS}/*.${SO}" )

  foreach( _lib ${_libs} )

    if ( WIN32 )

      install( FILES ${_lib} DESTINATION ${QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )

      find_library( maeparser_dll "maeparser"  HINTS "c:/opt/maeparser" "c:/maeparser" PATH_SUFFIXES "bin" "lib" )
      if ( maeparser_dll )
        install( FILES ${maeparser_dll} DESTINATION ${QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
      endif()

    else()

      get_filename_component( _name ${_lib} NAME_WE )
      get_filename_component( _path ${_lib} DIRECTORY )

      file( GLOB files "${_path}/${_name}.${SO}*" )

      #message( STATUS ${_path} " / " ${_name} " ==> " ${files} )
      install( PROGRAMS ${files} DESTINATION ${QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )

    endif()
  endforeach()

endif()
