# find QWT

if ( qwt_FOUND )
  return()
endif()

if ( WIN32 )
  if ( ${MSVC_VERSION} MATCHES "1900" ) # VS2015
    find_path( qwt_DIR NAMES include/qwt.h HINTS C:/Qwt-6.1.3-svn-vc14 $ENV{QWT} )
  else()
    find_path( qwt_DIR NAMES include/qwt.h HINTS C:/Qwt-6.1.3-svn-vc12 C:/Qwt-6.1.3-svn C:/Qwt-6.1.2-svn $ENV{QWT} )
  endif()
else()
  find_path( qwt_DIR NAMES include/qwt.h HINTS 
    /usr/local/qwt-6.1.3-svn
    /usr/local/qwt-6.1.2-svn
    /usr/local/qwt-6.1.2
    $ENV{QWT}
    )
endif()

if ( qwt_DIR )

  set( QWT_INCLUDE_DIR ${qwt_DIR}/include )
  set( QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR} )

  find_library( _release NAMES qwt HINTS ${qwt_DIR}/lib )
  find_library( _debug NAMES qwtd HINTS ${qwt_DIR}/lib )  

  if ( _release AND _debug )
    
    set( QWT_LIBRARIES
      debug ${_debug}
      optimized ${_release} )
    set( qwt_FOUND 1 )

  elseif( _release )

    set( QWT_LIBRARIES ${_release} )
    set( qwt_FOUND 1 )

  endif()

else()

  message( STATUS "QWT NOT Found" )

endif()
