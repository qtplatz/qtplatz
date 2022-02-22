# find QWT

if ( qwt_FOUND )
  return()
endif()

if ( WIN32 )
  set ( __prefix "C:" )
else()
  set ( __prefix "/usr/local" )
endif()

set ( __qwt_dirs "" )
foreach( __qwt_version "6.2.0" "6.1.6" "6.1.5" "6.1.4" "6.1.3" "6.1.2" )
  list ( APPEND __qwt_dirs "${__prefix}/qwt-${__qwt_version}" "${__prefix}/qwt-${__qwt_version}-svn" )
endforeach()

find_path( qwt_DIR NAMES include/qwt.h HINTS ${__qwt_dirs} "$ENV{QWT}" )

if ( qwt_DIR )
  set( QWT_INCLUDE_DIR "${qwt_DIR}/include" )
  set( QWT_INCLUDE_DIRS "${QWT_INCLUDE_DIR}" )

  set( QWT_LIB "QWT_LIB-NOTFOUND" )
  set( QWT_DEBUG_LIB "QWT_DEBUG_LIB-NOTFOUND" )

  find_library( QWT_LIB NAMES qwt HINTS ${qwt_DIR}/lib )

  if ( QWT_LIB )
    add_library( qwt STATIC IMPORTED )
    set ( QWT_LIBRARIES qwt )

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
    set( qwt_FOUND TRUE )
  else()
    message( FATAL_ERROR ${QWT_LIB} " " ${QWT_DLL} )
  endif()
endif()
