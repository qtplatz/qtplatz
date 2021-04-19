# find AgMD2 -- Keysight U5303A driver

set ( AqMD3_INCLUDE_DIR "AqMD3_INCLUDE_DIR-NOTFOUND" )
set ( _lib "_lib-NOTFOUND" )

if ( WIN32 )
  set ( CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll" )
  set ( IVIROOTDIR "C:/Program Files/IVI Foundation/IVI" )

  find_path( AqMD3_INCLUDE_DIR NAMES AqMD3.h PATHS "${IVIROOTDIR}/include" )

  find_library( _md3_lib NAMES AqMD3 HINTS    "${IVIROOTDIR}/Lib_x64/msc"  )
  find_library( _md3_dll NAMES AqMD3_64 HINTS "${IVIROOTDIR}/bin" )

  if ( _md3_lib AND _md3_dll )
    set( AqMD3_FOUND TRUE )
    add_library( AqMD3 SHARED IMPORTED )
    set_target_properties( AqMD3 PROPERTIES IMPORTED_IMPLIB "${_md3_lib}" IMPORTED_LOCATION "${_md3_dll}" )
    set( AqMD3_LIBRARIES AqMD3 )
  endif()

  find_library( _lio_lib NAMES AqLio HINTS    "${IVIROOTDIR}/Lib_x64/msc"  )
  find_library( _lio_dll NAMES AqLio_64 HINTS "${IVIROOTDIR}/bin" )
  if ( _lio_lib AND _lio_dll )
    add_library( AqLio SHARED IMPORTED )
    set_target_properties( AqLio PROPERTIES IMPORTED_IMPLIB "${_lio_lib}" IMPORTED_LOCATION "${_lio_dll}" )
    list ( APPEND AqMD3_LIBRARIES AqLio )
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
    list( APPEND AqMD3_LIBRARIES ${_lio} )
  endif()

endif()
