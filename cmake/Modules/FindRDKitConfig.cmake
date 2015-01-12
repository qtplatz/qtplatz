# find RDKit

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
  message( STATUS "RDKitConfig found in " ${rdkit_DIR} )
  find_package( rdkit )
else()
  message( STATUS ${rdkit_DIR} )
endif()
