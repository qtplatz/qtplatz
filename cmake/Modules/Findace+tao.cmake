
if ( ace+tao_FOUND )
  return()
endif()

if ( WIN32 )

  find_path( ace+tao_DIR NAMES ace/ACE.h HINTS $ENV{ACE_ROOT} C:/ACE_wrappers )

elseif( RTC_ARCH_ARM AND CMAKE_CROSSCOMPILING )

  find_path( ace+tao_DIR NAMES include/ace/ACE.h HINTS $ENV{ACE_ROOT}
    /usr/local/ace+tao/6.3.2
    /usr/local/ace+tao/6.3.1
    /usr/local/ace+tao/6.3.0 )
  string( REGEX REPLACE "(.*)/include" "\\1" ace+tao_DIR ${ace+tao_DIR} )

else()

  find_path( ace+tao_DIR NAMES ace/ACE.h include/ace/ACE.h HINTS $ENV{ACE_ROOT}
    /usr/local/ace+tao/6.3.2
    /usr/local/ace+tao/6.3.1
    /usr/local/ace+tao/6.3.0
    /usr/local/ace+tao/6.2.8 )

endif()

if ( ace+tao_DIR )

  set( ACE_ROOT ${ace+tao_DIR} )
  find_program( TAO_IDL NAMES tao_idl HINTS ${ACE_ROOT}/bin )

  message( STATUS "TAO_IDL : " ${TAO_IDL} )
  
  find_path( ACE_INCLUDE_DIR NAMES ace/ACE.h HINTS ${ace+tao_DIR} ${ace+tao_DIR}/include )
  find_path( TAO_INCLUDE_DIR NAMES tao/corba.h HINTS ${ace+tao_DIR}/TAO ${ace+tao_DIR} ${ace+tao_DIR}/include )

  if ( ACE_INCLUDE_DIR )
    set( ACE+TAO_INCLUDE_DIRS ${ACE_INCLUDE_DIR} )
  endif()
  if ( TAO_INCLUDE_DIR )
    set( ACE+TAO_INCLUDE_DIRS ${ACE+TAO_INCLUDE_DIRS} ${ACE_INCLUDE_DIR} )
  endif()

  file( STRINGS ${ACE_INCLUDE_DIR}/ace/Version.h ace_version REGEX "^#define[ \t]+ACE_VERSION" )
  string( REGEX REPLACE "#define[ \t]*ACE_VERSION[ \t]+\"(.*)\"" "\\1" ACE_VERSION ${ace_version} )

  file( STRINGS ${TAO_INCLUDE_DIR}/tao/Version.h tao_version REGEX "^#define[ \t]+TAO_VERSION" )
  string( REGEX REPLACE "#define[ \t]*TAO_VERSION[ \t]+\"(.*)\"" "\\1" TAO_VERSION ${tao_version} )

  set( ACE+TAO_LIBRARY_DIR  ${ace+tao_DIR}/lib )
  set( ACE+TAO_LIBRARY_DIRS ${ACE+TAO_LIBRARY_DIR} )
  
  include( ace+tao-config )

  set( ACE+TAO_LIBRARIES
    CORBA::TAO_Utils
    CORBA::TAO_PI
    CORBA::TAO_PortableServer
    CORBA::TAO_AnyTypeCode
    CORBA::TAO
    CORBA::ACE )

  if ( ACE_INCLUDE_DIR AND TAO_INCLUDE_DIR )
    set( ace+tao_FOUND TRUE )
    set( ACE+TAO_INCLUDE_DIRS ${ACE_INCLUDE_DIR} ${TAO_INCLUDE_DIR} )
  endif()
  message( STATUS "Found ACE VERSION " ${ACE_VERSION} " TAO VERSION " ${TAO_VERSION} " in " ${ace+tao_DIR} )
else()
  set( ace+tao_FOUND FALSE )
endif()



