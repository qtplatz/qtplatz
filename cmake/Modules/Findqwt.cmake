# find QWT

if ( qwt_FOUND )
  return()
endif()

if ( WIN32 )
  find_path( qwt_DIR NAMES include/qwt.h HINTS C:/qwt-6.1.4-svn C:/Qwt-6.1.3 $ENV{QWT} )
else()
  find_path( qwt_DIR NAMES include/qwt.h HINTS
    /usr/local/qwt-6.1.4-svn
    /usr/local/qwt-6.1.3-svn
    /usr/local/qwt-6.1.2-svn
    /usr/local/qwt-6.1.2
    $ENV{QWT}
    )
endif()

if ( qwt_DIR )
  set( QWT_INCLUDE_DIR ${qwt_DIR}/include )
  set( QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR} )

  set( QWT_LIB "QWT_LIB-NOTFOUND" )
  set( QWT_DEBUG_LIB "QWT_DEBUG_LIB-NOTFOUND" )

  find_library( QWT_LIB NAMES qwt HINTS ${qwt_DIR}/lib )
#  find_library( QWT_DEBUG_LIB NAMES qwt${CMAKE_DEBUG_POSTFIX} HINTS ${qwt_DIR}/lib )    
  if ( QWT_LIB )
    add_library( Qwt STATIC IMPORTED )
    set ( QWT_LIBRARIES Qwt )
    set_target_properties( Qwt PROPERTIES IMPORTED_LOCATION ${QWT_LIB} )
    if ( QWT_DEBUG_LIB )
      set_target_properties( Qwt PROPERTIES IMPORTED_LOCATION_DEBUG ${QWT_DEBUG_LIB} )
    endif()
    set( qwt_FOUND TRUE )
  else()
    message( FATAL_ERROR ${QWT_LIB} " " ${QWT_DLL} )
  endif()
endif()
