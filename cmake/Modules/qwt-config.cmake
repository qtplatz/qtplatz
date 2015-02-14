# find QWT

if( NOT _defined )

  set( _defined 1 )
  
  add_library( qwt STATIC IMPORTED )

  if ( WIN32 ) 
    set_target_properties( qwt PROPERTIES
      IMPORTED_LOCATION ${qwt_DIR}/lib/qwt.lib
      IMPORTED_LOCATION_DEBUG ${qwt_DIR}/lib/qwtd.lib
      )
  else()
    set_target_properties( qwt PROPERTIES
      IMPORTED_LOCATION ${qwt_DIR}/lib/libqwt.a
      )
  endif()

endif()
