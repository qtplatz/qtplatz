# find RDKit

if ( rdkit_FOUND )
  return()
endif()

if ( WIN32 )
  if ( RTC_ARCH_X86 )
    #Win32
    find_path( rdkit_config_DIR NAMES rdkit-config.cmake PATHS
      $ENV{RDBASE}/build_x86_120
      $ENV{HOME}/src/rdkit/build_x86_120
      $ENV{USERPROFILE}/src/rdkit/build_x86_120 )
  else()
    #Win64
    find_path( rdkit_config_DIR NAMES rdkit-config.cmake PATHS
      $ENV{RDBASE}/build_x86_64_120
      $ENV{HOME}/src/rdkit/build_x86_64_120
      $ENV{USERPROFILE}/src/rdkit/build_x86_64_120
      )
  endif()

else()

  find_path( rdkit_config_DIR NAMES rdkit-config.cmake PATHS
    $ENV{RDBASE}/build
    $ENV{HOME}/src/rdkit/build
    /usr/local/rdkit
    /usr/local/lib )

  if ( ${rdkit_config_DIR} MATCHES "rdkit_config_DIR-NOTFOUND" )
    # find apt-get'ed module
    find_path( RDKIT_INCLUDE_DIR GraphMol/RDKitBase.h PATHS /usr/include/rdkit )
    if ( ${RDKIT_INCLUDE_DIR} MATCHES "RDKIT_INCLUDE_DIR-NOTFOUND" )
      message( STATUS ${RDKIT_INCLUDE_DIR} )
      return()
    endif()
    find_library( RDKIT_FILEPARSERS_LIB NAMES FileParsers PATHS ${RDBASE}/lib )
    if ( RDKIT_FILEPARSERS_LIB )
      get_filename_component ( RDKIT_LIBRARY_DIR ${RDKIT_FILEPARSERS_LIB} PATH )
      message( STATUS "Found RDKIT libraries at ${RDKIT_LIBRARY_DIR}" )
    endif()
  endif()

endif()

if ( rdkit_config_DIR )

  set ( rdkit_FOUND 1 )
  set ( RDKIT_FOUND 1 )    
  message( "## FOUND rdkit-config: " ${rdkit_config_DIR} )  

  if ( APPLE )
    get_filename_component (_prefix "${rdkit_config_DIR}/.." ABSOLUTE)
    set (RDKIT_INCLUDE_DIRS "${_prefix}/Code")
    include( rdkit-darwin-config )
  else()
    find_package( rdkit CONFIG )
    message( STATUS "RDKIT Version: " ${rdkit_VERSION} " in " ${rdkit_config_DIR} )
  endif()

endif()

if ( RDKIT_INCLUDE_DIR AND RDKIT_LIBRARY_DIR )
  
  set ( rdkit_FOUND 1 )
  set ( RDKIT_FOUND 1 )  
  set ( RDKIT_INCLUDE_DIRS ${RDKIT_INCLUDE_DIR} )

  find_library(SMILESPARSE_LIB   NAMES SmilesParse   HINTS ${RDKIT_LIBRARY_DIR})
  find_library(DEPICTOR_LIB      NAMES Depictor      HINTS ${RDKIT_LIBRARY_DIR})
  find_library(GRAPHMOL_LIB      NAMES GraphMol      HINTS ${RDKIT_LIBRARY_DIR})
  find_library(RDGEOMETRYLIB_LIB NAMES RDGeometryLib HINTS ${RDKIT_LIBRARY_DIR})
  find_library(RDGENERAL_LIB     NAMES RDGeneral     HINTS ${RDKIT_LIBRARY_DIR})

  set (RDKIT_LIBRARIES
    ${FILEPARSERS_LIB}
    ${SMILESPARSE_LIB}
    ${DEPICTOR_LIB}
    ${GRAPHMOL_LIB}
    ${RDGEOMETRYLIB_LIB}
    ${RDGENERAL_LIB})  
  
  message( STATUS "## RDKIT FOUND ##" )
  
endif()
