
if ( WIN32 )
  # set( VXIPNPPATH "C:/Program Files/IVI Foundation/VISA" )
  set( IVIROOTDIR "C:/Program Files/IVI Foundation/IVI" )
  find_path( AgMD2_INCLUDE_DIR NAME AgMD2.h PATHS ${IVIROOTDIR}/include )

  set( ACQRSDIR   "C:/Program Files (x86)/Agilent/Acqiris" )
  find_path( AqDrv4_INCLUDE_DIR NAME AcqirisImport.h PATHS "${ACQRSDIR}/include" )

  if ( NOT AgMD2_INCLUDE_DIR )
    set( AgMD2_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/contrib/agilent/include )
  endif()

  include_directories( ${AgMD2_INCLUDE_DIR} )

elseif( APPLE )
  
  find_path( AqDrv4_INCLUDE_DIR NAME AcqirisImport.h PATHS ${CMAKE_SOURCE_DIR}/contrib/agilent/include )
  if ( AqDrv4_INCLUDE_DIR )
    add_definitions(-D_LINUX -D_ACQIRIS )
    include_directories( ${AqDrv4_INCLUDE_DIR} )
  else()
    message( FATAL_ERROR "### AqDrv4_INCLUDE_DIR: " ${AqDrv4_INCLUDE_DIR} )
  endif()
  
else() # Linux
  
  find_path( AgMD2_INCLUDE_DIR NAME AgMD2.h PATHS /usr/include )
  find_path( AqDrv4_INCLUDE_DIR NAME AcqirisImport.h PATHS /usr/include ${CMAKE_SOURCE_DIR}/contrib/agilent/include )
  find_library( AqDrv4_LIBRARY AqDrv4 )
  if ( AgMD2_INCLUDE_DIR )
    set( WITH_AP240 ON )
  else()
    find_path( AqDrv4_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/contrib/agilent/include )
  endif()
  
endif()

add_subdirectory( libs )
add_subdirectory( plugins )

if ( AgMD2_INCLUDE_DIR )
  # add_subdirectory( test_u5303a )
endif()
