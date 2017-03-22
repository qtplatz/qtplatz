
if ( WIN32 )
  set( SO "dll")
elseif( APPLE )
  set( SO "dylib" )
else()
  set( SO "so" )
endif()
