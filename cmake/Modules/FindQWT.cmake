# find QWT

if ( WIN32 )
  find_path( qwt_DIR NAMES include/qwt.h HINTS $ENV{QWT} C:/Qwt-6.1.2-svn C:/Qwt-6.1.3-svn )
else()
  find_path( qwt_DIR NAMES include/qwt.h HINTS $ENV{QWT} /usr/local/qwt-6.1.3-svn /usr/local/qwt-6.1.2-svn )
endif()

if ( qwt_DIR )
  message( STATUS "QWT Found in : " ${qwt_DIR} )
  find_path( QWT_INCLUDE_DIR NAMES qwt.h HINTS ${qwt_DIR}/include )
  find_library( QWT_LIBRARY NAMES qwt HINTS ${qwt_DIR}/lib )
  find_library( QWT_LIBRARY_DEBUG NAMES qwtd HINTS ${qwt_DIR}/lib )

  set( QWT_INCLUDE_DIRS ${QWT_INCLUDE_DIR} )
  set( QWT_LIBRARIES ${QWT_LIBRARY} )

else()

  message( STATUS "QWT NOT Found" )

endif()
