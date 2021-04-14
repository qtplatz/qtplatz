# find AgMD2 -- Keysight U5303A driver

set ( AqMD3_INCLUDE_DIR "AqMD3_INCLUDE_DIR-NOTFOUND" )
set ( _lib "_lib-NOTFOUND" )

if ( WIN32 )
  set ( CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll" )
  set ( IVIROOTDIR "C:/Program Files/IVI Foundation/IVI" )

  find_path( AqMD3_INCLUDE_DIR NAMES AqMD3.h PATHS "${IVIROOTDIR}/include" )

  find_library( _lib NAMES AqMD3 HINTS "${IVIROOTDIR}/Lib_x64/msc"  )
  set ( _dll "_dll-NOTFOUND" )
  find_library( _dll NAMES AqMD3_64 HINTS "${IVIROOTDIR}/bin" )

  if ( _lib AND _dll )
    set( AqMD3_FOUND TRUE )
    add_library( AqMD3 SHARED IMPORTED )
    set_target_properties( AqMD3 PROPERTIES IMPORTED_IMPLIB "${_lib}" IMPORTED_LOCATION "${_dll}" )
    set( AqMD3_LIBRARIES AqMD3 )
  endif()

elseif( APPLE )

  set( AqMD3_FOUND FALSE )
  # no driver supported

else() # Linux

  set( AqMD3_FOUND FALSE )
  find_path( AqMD3_INCLUDE_DIR NAMES AqMD3.h PATHS /usr/include )

  set ( AqMD3_LIBRARIES )

  find_library( _md3 NAMES AqMD3 )
  if ( _md3 )
    set( AqMD3_FOUND TRUE )
    list( APPEND AqMD3_LIBRARIES ${_md3} )
  endif()

  find_library( _lio NAMES AqLio )
  if ( _lio )
    set( AqMD3_FOUND TRUE )
    list( APPEND AqMD3_LIBRARIES ${_lio} )
  endif()

endif()
