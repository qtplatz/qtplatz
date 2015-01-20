
if ( NOT ace+tao_FOUND )

  if ( WIN32 )
    find_path( ace+tao_DIR NAMES ace/ace.h HINTS ENV ACE_ROOT C:/ACE_wrappers )
  else()
    find_path( ace+tao_DIR NAMES ace/ace.h include/ace/ace.h HINTS ENV ACE_ROOT /usr/local/ace+tao/6.3.0 /usr/local/ace+tao/6.2.8 )
  endif()

  if ( ace+tao_DIR )

    set( ace+tao_FOUND 1 )
    set( ACE+TAO_LIBRARY_DIR  ${ace+tao_DIR}/lib )
    set( ACE+TAO_LIBRARY_DIRS ${ACE+TAO_LIBRARY_DIR} )

    find_path( ACE_INCLUDE_DIR NAMES ace/ace.h HINTS ${ace+tao_DIR} ${ace+tao_DIR}/include )
    find_path( TAO_INCLUDE_DIR NAMES tao/corba.h HINTS ${ace+tao_DIR}/TAO ${ace+tao_DIR} ${ace+tao_DIR}/include )

    if ( ACE_INCLUDE_DIR )
      set( ACE+TAO_INCLUDE_DIRS ${ACE_INCLUDE_DIR} )
    endif()
    if ( TAO_INCLUDE_DIR )
      set( ACE+TAO_INCLUDE_DIRS ${ACE+TAO_INCLUDE_DIRS} ${ACE_INCLUDE_DIR} )
    endif()

    include( ace+tao-config )

    set( ACE+TAO_LIBRARIES
      CORBA::TAO_Utils
      CORBA::TAO_PI
      CORBA::TAO_PortableServer
      CORBA::TAO_AnyTypeCode
      CORBA::TAO
      CORBA::ACE )

#    set( ACE+TAO_LIBRARIES 
#      optimized TAO_Utils debug TAO_Utilsd
#      optimized TAO_PI debug TAO_PId
#      optimized TAO_PortableServer debug TAO_PortableServerd
#      optimized TAO_AnyTypeCode debug TAO_AnyTypeCoded
#      optimized TAO debug TAOd
#      optimized ACE debug ACEd )
    
    if ( ACE_INCLUDE_DIR AND TAO_INCLUDE_DIR )
      set( ace+tao_FOUND 1 )
      set( ACE+TAO_INCLUDE_DIRS ${ACE_INCLUDE_DIR} ${TAO_INCLUDE_DIR} )
    endif()
    message( STATUS "ACE+TAO Found in " ${ace+tao_DIR} )
  else()
    message( STATUS "ACE+TAO NOT Found" )
  endif()

endif()

#include_directories( ${ACE_INCLUDE_DIR} ${TAO_INCLUDE_DIR} )
#link_directories( ${TAO_LIBRARY_DIR} )


