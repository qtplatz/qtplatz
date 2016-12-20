# find AqDrv4 -- Keysight AP240/DC110... driver

if ( WIN32 )

  set( ACQRSDIR   "C:/Program Files (x86)/Agilent/Acqiris" )
  find_path( AqDrv4_INCLUDE_DIR NAME AcqirisImport.h PATHS "${ACQRSDIR}/include" )

  find_library( _lib NAME AqDrv4 PATHS "${ACQRSDIR}/bin" )
  if ( _lib )
    set( AqDrv4_FOUND TRUE )
    set( AqDrv4_LIBRARIES ${_lib} )
  endif()

elseif( APPLE )

  find_path( AqDrv4_INCLUDE_DIR NAME AcqirisImport.h PATHS ${CMAKE_SOURCE_DIR}/contrib/agilent/include )
  
else() # Linux
  
  find_path( AqDrv4_INCLUDE_DIR NAME AcqirisImport.h PATHS /usr/include ${CMAKE_SOURCE_DIR}/contrib/agilent/include )
  find_library( AqDrv4_LIBRARY AqDrv4 )
  if ( AqDrv4_LIBRARY )
    set( AqDrv4_FOUND TRUE )
  endif()

endif()

