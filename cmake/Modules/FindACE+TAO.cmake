
if ( NOT ace+tao_FOUND )

  if ( WIN32 )
    find_path( ace+tao_DIR NAMES ace/ace.h HINTS ENV ACE_ROOT C:/ACE_wrappers )
  else()
    find_path( ace+tao_DIR NAMES ace/ace.h HINTS ENV ACE_ROOT /usr/local/ace+tao/6.2.8 )
  endif()

  if ( ace+tao_DIR )

    find_path( ACE_INCLUDE_DIR NAMES ace/ace.h HINTS ${ace+tao_DIR} )
    find_path( TAO_INCLUDE_DIR NAMES tao/corba.h HINTS ${ace+tao_DIR}/TAO ${ace+tao_DIR} )

    if ( TAO_INCLUDE_DIR AND TAO_INCLUDE_DIR )
      set( ace+tao_FOUND 1 )
      set( ACE+TAO_INCLUDE_DIRS ${ACE_INCLUDE_DIR} ${TAO_INCLUDE_DIR} )
      set( ACE+TAO_LIBRARY_DIR  ${ace+tao_DIR}/lib )
      set( ACE+TAO_LIBRARY_DIRS ${ACE+TAO_LIBRARY_DIR} )
    endif()

    message( STATUS "ACE+TAO Found in " ${ace+tao_DIR} )
  else()
    message( STATUS "ACE+TAO NOT Found" )
  endif()

endif()

#include_directories( ${ACE_INCLUDE_DIR} ${TAO_INCLUDE_DIR} )
#link_directories( ${TAO_LIBRARY_DIR} )


