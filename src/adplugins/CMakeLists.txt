
add_subdirectory( addatafile )
add_subdirectory( adtextfile )
add_subdirectory( adspectrometer )
#add_subdirectory( adtofprocessor )
if ( netCDF_FOUND )
  add_subdirectory( adnetcdf )
endif()
