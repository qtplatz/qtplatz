
if ( WIN32 )
  # set( VXIPNPPATH "C:/Program Files/IVI Foundation/VISA" )
  set( IVIROOTDIR "C:/Program Files/IVI Foundation/IVI" )
  find_path( AgMD2_INCLUDE_DIR NAME AgMD2.h PATHS ${IVIROOTDIR}/include )

  set( ACQRSDIR   "C:/Program Files (x86)/Agilent/Acqiris" )
  find_path( AqDrv4_INCLUDE_DIR NAME AcqirisImport.h PATHS "${ACQRSDIR}/include" )

else()
  
  find_path( AgMD2_INCLUDE_DIR NAME AgMD2.h PATHS /usr/include )
  find_path( AqDrv4_INCLUDE_DIR NAME AcqirisImport.h PATHS /usr/include )
  
endif()

message( STATUS "### AgMD2  : " ${AgMD2_INCLUDE_DIR} )
message( STATUS "### AqDrv4 : " ${AqDrv4_INCLUDE_DIR} )

add_subdirectory( libs )
add_subdirectory( plugins )

if ( AgMD2_INCLUDE_DIR )
    add_subdirectory( test_u5303a )
endif()
