# find libxml2

if ( WIN32 )
  return()
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES Darwin )

#  239  brew install libxml2 libxslt libiconv
#  240  brew link --force libxml2
#  241  brew link --force libxslt

  find_path( libxml2_INCLUDE_DIR NAMES libxml/xpath.h PATHS /usr/include/libxml2 /opt/local/include/libxml2 /usr/local/opt/libxml2/include/libxml2 )
  find_library( libxml2_LIBRARY NAMES xml2 libxml2 PATHS /usr/lib /opt/local /usr/local/opt/libxml2/lib )

elseif (${CMAKE_SYSTEM_NAME} MATCHES Linux )

  find_path( libxml2_INCLUDE_DIR NAMES libxml/xpath.h PATHS /usr/include/libxml2 /usr/local/include/libxml2 )
  find_library( libxml2_LIBRARY NAMES xml2 libxml2 PATHS /usr/lib )

endif()

if ( libxml2_INCLUDE_DIR )
  set( libxml2_FOUND 1 )
  set( libxml2_INCLUDE_DIRS ${libxml2_INCLUDE_DIR} )
  set( libxml2_LIBRARIES ${libxml2_LIBRARY} )
endif()

if ( NOT libxml2_FOUND )
  message( STATUS "libxml2 not find" )
endif()

