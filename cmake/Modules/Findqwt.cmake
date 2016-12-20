# find QWT

if ( qwt_FOUND )
  return()
endif()

if ( WIN32 )
  find_path( qwt_DIR NAMES include/qwt.h HINTS C:/Qwt-6.1.3 C:/qwt-6.1.4-svn $ENV{QWT} )
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
  find_library( QWT_DEBUG_LIB NAMES qwt${CMAKE_DEBUG_POSTFIX} HINTS ${qwt_DIR}/lib )

  if ( QWT_LIB AND QWT_DEBUG_LIB )
    
    set( QWT_LIBRARIES debug ${QWT_DEBUG_LIB} optimized ${QWT_LIB} )
    set( qwt_FOUND 1 )

  elseif( QWT_LIB )

    set( QWT_LIBRARIES ${QWT_LIB} )
    set( qwt_FOUND 1 )

  else()
    message( FATAL_ERROR "QWT NOT Found" )
  endif()

else()

  message( STATUS "QWT NOT Found" )

endif()
