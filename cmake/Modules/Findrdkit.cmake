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
  if ( APPLE )
    #include( ${rdkit_DIR}/rdkit-config.cmake )
    #include( ${rdkit_DIR}/CMakeFiles/Export/lib/rdkit-targets.cmake )
    get_filename_component (_dir ${rdkit_DIR}/rdkit-config.cmake PATH)
    get_filename_component (_prefix "${_dir}/.." ABSOLUTE)
    #set (RDKit_INCLUDE_DIRS "${_prefix}/include/rdkit")    # if INSTALL_INTREE is OFF
    set (RDKit_INCLUDE_DIRS "${_prefix}/Code")
  else()
    find_package( rdkit CONFIG )
    message( STATUS "RDKit version: " ${rdkit_VERSION} " in " ${rdkit_DIR} )
  endif()
else()
  message( STATUS ${rdkit_DIR} )
endif()
