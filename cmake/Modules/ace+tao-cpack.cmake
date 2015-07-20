
find_package( ace+tao )

include( "soname" )

if ( ace+tao_FOUND )

  set ( libs ACE TAO TAO_Utils TAO_PI TAO_PortableServer TAO_AnyTypeCode )

  foreach( lib ${libs} )
    file( GLOB _libs ${ace+tao_DIR}/lib/lib${lib}.${SO}* )
    install( PROGRAMS ${_libs} DESTINATION ${dest} COMPONENT runtime_libraries )
  endforeach()
  
endif()