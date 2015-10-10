
add_subdirectory( addatafile )
add_subdirectory( adtextfile )

if ( QTPLATZ_SUPPORT_CORBA )
  add_subdirectory( adbroker )
  add_subdirectory( adcontroller )
endif()

