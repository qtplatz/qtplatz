
include( "soname" )

find_package( ace+tao )

if ( ace+tao_FOUND )

  set ( libs ACE TAO TAO_Utils TAO_PI TAO_PortableServer TAO_AnyTypeCode TAO_CodecFactory )

  foreach( lib ${libs} )
    
    if ( WIN32 )
      file( GLOB _libs ${ACE_ROOT}/lib/${lib}.${SO} )
    else()
      file( GLOB _libs ${ACE_ROOT}/lib/lib${lib}.${SO}* )
    endif()
    
    install( PROGRAMS ${_libs} DESTINATION ${dest} COMPONENT runtime_libraries )
    message( STATUS "## ace+tao_cpack: lib " ${_libs} " --> " ${dest} )    

  endforeach()
  
endif()