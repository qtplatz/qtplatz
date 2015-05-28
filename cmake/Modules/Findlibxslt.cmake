# find libxml2

if ( WIN32 )
  return()
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES Darwin )
  
  find_path( libxslt_INCLUDE_DIR NAMES libxslt/xslt.h PATHS /usr/include /opt/local/include )
  find_library( libxslt_LIBRARY NAMES xslt libxslt PATHS /usr/lib )

elseif (${CMAKE_SYSTEM_NAME} MATCHES Linux )

  find_path( libxslt_INCLUDE_DIR NAMES libxslt/xslt.h PATHS /usr/include /usr/local/include )
  find_library( libxslt_LIBRARY NAMES xslt libxslt PATHS /usr/lib )

endif()

if ( libxslt_INCLUDE_DIR )
  set( libxslt_FOUND 1 )
  set( libxslt_INCLUDE_DIRS ${libxslt_INCLUDE_DIR} )
  set( libxslt_LIBRARIES ${libxslt_LIBRARY} )  
endif()
