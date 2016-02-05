
if ( WIN32 )
  set( IVIROOTDIR "C:/Program Files/IVI Foundation/IVI" )
  set( VXIPNPPATH "C:/Program Files/IVI Foundation/VISA" )
  set( ACQRSDIR   "C:/Program Files (x86)/Agilent/Acqiris" )
endif()

find_path( AgMD2_INCLUDE_DIR NAME AgMD2.h PATHS /usr/include ${IVIROOTDIR} ${VXIPNPPATH} )
find_path( AqDrv4_INCLUDE_DIR NAME AcqirisImport.h PATHS /usr/include "${ACQRSDIR}/include" )

add_subdirectory( libs )
add_subdirectory( plugins )
if ( NOT APPLE )
  add_subdirectory( test_u5303a )
endif()



