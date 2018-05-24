# find AqDrv4 -- Keysight AP240/DC110... driver

if ( WIN32 )
  set( ACQRSDIR   "C:/Program Files (x86)/Agilent/Acqiris" )
  find_path( AqDrv4_INCLUDE_DIR NAMES AcqirisImport.h PATHS "${ACQRSDIR}/include" )

  set( _lib "_lib-NOTFOUND" )
  set( _dll "_dll-NOTFOUND" )
  set ( CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll")
  find_library( _lib NAMES AqDrv4_x64.lib PATHS "${ACQRSDIR}/lib" PATH_SUFFIXES ".lib" )
  find_library( _dll NAMES AqDrv4_x64.dll PATHS "${ACQRSDIR}/bin" )

  if ( _lib AND _dll )
    set( AqDrv4_FOUND TRUE )
    add_library( AqDrv4 SHARED IMPORTED )
    set_target_properties( AqDrv4 PROPERTIES IMPORTED_IMPLIB "${_lib}" IMPORTED_LOCATION "${_dll}")
    set( AqDrv4_LIBRARIES AqDrv4 )
  else()
    find_path( AqDrv4_INCLUDE_DIR NAMES AcqirisImport.h PATHS ${CMAKE_SOURCE_DIR}/contrib/agilent/include )
  endif()

elseif( APPLE )

  find_path( AqDrv4_INCLUDE_DIR NAMES AcqirisImport.h PATHS ${CMAKE_SOURCE_DIR}/contrib/agilent/include )
  set( AqDrv4_FOUND FALSE )
  
else() # Linux
  
  find_path( AqDrv4_INCLUDE_DIR NAMES AcqirisImport.h PATHS ${CMAKE_SOURCE_DIR}/contrib/agilent/include )

  set( _lib "_lib-NOTFOUND" )  
  find_library( _lib AqDrv4 )
  if ( _lib )
    set( AqDrv4_FOUND TRUE )
    set( AqDrv4_LIBRARIES ${_lib} )    
  endif()

endif()

