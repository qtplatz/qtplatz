# find AgMD2 -- Keysight U5303A driver

set ( AgMD2_INCLUDE_DIR "AgMD2_INCLUDE_DIR-NOTFOUND" )
set ( _lib "_lib-NOTFOUND" )

if ( WIN32 )
  set ( CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll" )
  set ( IVIROOTDIR "C:/Program Files/IVI Foundation/IVI" )

  find_path( AgMD2_INCLUDE_DIR NAMES AgMD2.h PATHS "${IVIROOTDIR}/include" )

  find_library( _lib NAMES AgMD2 HINTS "${IVIROOTDIR}/Lib_x64/msc"  )
  set ( _dll "_dll-NOTFOUND" )
  find_library( _dll NAMES AgMD2_64 HINTS "${IVIROOTDIR}/bin" )

  if ( _lib AND _dll )
    set( AgMD2_FOUND TRUE )
    add_library( AgMD2 SHARED IMPORTED )
    set_target_properties( AgMD2 PROPERTIES IMPORTED_IMPLIB "${_lib}" IMPORTED_LOCATION "${_dll}" )
    set( AgMD2_LIBRARIES AgMD2 )
  endif()
  
elseif( APPLE )

  set( AgMD2_FOUND FALSE )  
  # no driver supported
  
else() # Linux
  
  find_path( AgMD2_INCLUDE_DIR NAMES AgMD2.h PATHS /usr/include )
  find_library( _lib NAMES AgMD2 )
  if ( _lib )
    set( AgMD2_FOUND TRUE )
    set( AgMD2_LIBRARIES ${_lib} )    
  endif()    

endif()

