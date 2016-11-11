# find AqDrv4 -- Keysight AP240/DC110... driver

if ( WIN32 )
  #set( VXIPNPPATH "C:/Program Files/IVI Foundation/VISA" )
  set( IVIROOTDIR "C:/Program Files/IVI Foundation/IVI" )
  find_path( AgMD2_INCLUDE_DIR NAME AgMD2.h PATHS ${IVIROOTDIR}/include )
  
  if ( AqMD2_INCLUDE_DIR )
    set( AgMD2_FOUND TRUE )
    set( AqMD2_LIBRARIES AqMD2 )
  endif()

elseif( APPLE )

  # no driver supported
  
else() # Linux
  
  find_path( AgMD2_INCLUDE_DIR NAME AgMD2.h PATHS /usr/include )
  find_library( AqMD2_LIBRARY AqMD2 )
  if ( AqMD2_LIBRARY )
    set( AgMD2_FOUND TRUE )
  endif()

endif()

