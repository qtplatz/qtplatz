# find QWT

if ( qwt_FOUND )
  return()
endif()

if ( WIN32 )
  find_path( qwt_DIR NAMES include/qwt.h HINTS $ENV{QWT} C:/Qwt-6.1.3-svn C:/Qwt-6.1.2-svn )
else()
  find_path( qwt_DIR NAMES include/qwt.h HINTS $ENV{QWT} /usr/local/qwt-6.1.3-svn /usr/local/qwt-6.1.2-svn )
endif()

if ( qwt_DIR )

  set( QWT_INCLUDE_DIR ${qwt_DIR}/include )
  set( QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR} )

  find_library( QWT_LIBRARIES NAMES qwt HINTS ${qwt_DIR}/lib )

  if ( QWT_LIBRARIES )
  
    set( qwt_FOUND 1 )
    message( STATUS "qwt_DIR : " ${qwt_DIR} )
    message( STATUS "QWT_LIBRARIES : " ${QWT_LIBRARIES} )

  endif()

else()

  message( STATUS "QWT NOT Found" )

endif()
