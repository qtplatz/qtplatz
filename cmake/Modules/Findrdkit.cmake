# find RDKit

if ( rdkit_FOUND )
  return()
endif()

if ( WIN32 )
  if ( RTC_ARCH_X86 )
    #Win32
    find_path( rdkit_DIR NAMES rdkit-config.cmake PATHS $ENV{RDBASE}/build_x86_120 $ENV{HOME}/src/rdkit/build_x86_120 )
  else()
    #Win64
    find_path( rdkit_DIR NAMES rdkit-config.cmake PATHS $ENV{RDBASE}/build_x86_64_120 $ENV{HOME}/src/rdkit/build_x86_64_120 )
  endif()

else()

  find_path( rdkit_DIR NAMES rdkit-config.cmake PATHS $ENV{RDBASE}/build $ENV{HOME}/src/rdkit/build /usr/local/rdkit ) 

endif()

if ( rdkit_DIR )

  set( rdkit_FOUND 1 )

  if ( APPLE )
    get_filename_component (_prefix "${rdkit_DIR}/.." ABSOLUTE)
    set (RDKit_INCLUDE_DIRS "${_prefix}/Code")
    include( rdkit-darwin-config )
  else()
    find_package( rdkit CONFIG )
    message( STATUS "RDKit version: " ${rdkit_VERSION} " in " ${rdkit_DIR} )
  endif()
else()
  message( STATUS ${rdkit_DIR} )
endif()
