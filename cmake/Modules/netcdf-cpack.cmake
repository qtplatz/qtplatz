
if ( NOT WIN32 )
  return()
endif()

message( STATUS "########################################################" )
message( STATUS "############ netCDF CPACK -- netCDF ${netCDFVersion} ############" )

#set( dependent_dlls "zlib1.dll" "zlib1.dll" )

get_target_property( __dll netCDF::netcdf IMPORTED_LOCATION_RELEASE )
message( STATUS "output --> ${dll}" )

cmake_path( GET __dll PARENT_PATH __path )

install ( PROGRAMS ${__dll} DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
set ( __dlls ${__dll} )
foreach ( __var ${dependent_dlls} )
  list( APPEND __dlls "${__path}/${__var}" )
endforeach()

foreach( __var ${__dlls} )
  message( STATUS "############ netCDF CPACK -- netCDF install ${__var} ---> ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY}" )
endforeach()

install ( PROGRAMS ${__dlls} DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
