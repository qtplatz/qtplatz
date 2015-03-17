# find QWT

if ( qwt_FOUND )
  return()
endif()

if ( WIN32 )
  find_path( qwt_DIR NAMES include/qwt.h HINTS $ENV{QWT} C:/Qwt-6.1.2-svn C:/Qwt-6.1.3-svn )
else()
  find_path( qwt_DIR NAMES include/qwt.h HINTS $ENV{QWT} /usr/local/qwt-6.1.3-svn /usr/local/qwt-6.1.2-svn )
endif()

if ( qwt_DIR )

  set( QWT_INCLUDE_DIR ${qwt_DIR}/include )
  set( QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR} )

  include( qwt-config )
  set( QWT_LIBRARIES qwt )

  set( qwt_FOUND 1 )

  message( STATUS "QWT Found in : " ${qwt_DIR} )

else()

  message( STATUS "QWT NOT Found" )

endif()
