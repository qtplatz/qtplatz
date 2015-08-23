# find RDKit

if ( rdkit_FOUND )
  return()
endif()

set( rdkit_FOUND FALSE )

if ( WIN32 )

  if ( RTC_ARCH_X86 )
    set( w32out_dir "build_x86_120" )
  else()
    set( w32out_dir "build_x86_64_120" )
  endif()

  # when run INSTALL.vcxproj, .libs are copied into RDKit's ${CMAKE_SOURCE_DIR}/rdkit/lib
  find_package( rdkit CONFIG PATHS
    $ENV{RDBASE}/lib
    $ENV{HOME}/src/rdkit/lib
    ${CMAKE_SOURCE_DIR}/../rdkit/lib
    $ENV{RDBASE}/${w32out_dir}
    $ENV{HOME}/src/rdkit/${w32out_dir}
    ${CMAKE_SOURCE_DIR}/../rdkit/${w32out_dir}
    )

  message( STATUS "### Find rdkit-config.cmake: " ${rdkit} )

else()

  find_package( rdkit CONFIG PATHS
    /usr/local/lib
    ${CMAKE_BINARY_DIR}/rdkit
    $ENV{RDBASE}
    $ENV{RDBASE}/build )

endif()

if ( rdkit )
  
  set (RDKit_LIBRARIES
    FileParsers
    SmilesParse
    Depictor
    GraphMol
    RDGeometryLib
    RDGeneral
    SubstructMatch )  

  set( rdkit_FOUND TRUE )
  return()
  
endif()

find_path( _include_dir  GraphMol/RDKitBase.h PATHS
  $ENV{RDBASE}/Code
  /usr/local/include/rdkit
  /usr/include/rdkit
  )

if ( _include_dir )
  set ( RDKit_INCLUDE_DIRS ${_include_dir} )
  get_filename_component ( rdbase ${_include_dir} PATH )
  if ( ${rdbase} MATCHES "/usr/include" )
    set( rdbase "/usr" )
  endif()
  if ( ${rdbase} MATCHES "/usr/local/include" )
    set( rdbase "/usr/local" )
  endif()
else()
  return()
endif()

find_library( _fileparsers_lib NAMES FileParsers PATHS
  $ENV{RDBASE}/lib
  $ENV{RDBASE}/build/lib
  /usr/local/lib
  /usr/lib )

if ( _fileparsers_lib )
  get_filename_component ( _libdir ${_fileparsers_lib} PATH )
  message( STATUS "Found rdkit libraries at ${_libdir}" )
endif()

if ( _include_dir AND _libdir )
  
  set ( rdkit_FOUND TRUE )
  set ( RDKit_INCLUDE_DIRS ${_include_dir} )

  find_library(SMILESPARSE_LIB   NAMES SmilesParse   HINTS ${_libdir})
  find_library(DEPICTOR_LIB      NAMES Depictor      HINTS ${_libdir})
  find_library(GRAPHMOL_LIB      NAMES GraphMol      HINTS ${_libdir})
  find_library(RDGEOMETRYLIB_LIB NAMES RDGeometryLib HINTS ${_libdir})
  find_library(RDGENERAL_LIB     NAMES RDGeneral     HINTS ${_libdir})
  find_library(SUBSTRUCTMATCH_LIB NAMES SubstructMatch HINTS ${_libdir})

  set (RDKit_LIBRARIES
    FileParsers
    SmilesParse
    Depictor
    Descriptors
    GraphMol
    RDGeometryLib
    RDGeneral
    SubstructMatch
    )
  
#  set (RDKit_LIBRARIES
#    ${_fileparsers_lib}
#    ${SMILESPARSE_LIB}
#    ${DEPICTOR_LIB}
#    ${GRAPHMOL_LIB}
#    ${RDGEOMETRYLIB_LIB}
#    ${RDGENERAL_LIB}
#    ${SUBSTRUCTMATCH_LIB}
#    )  
  
endif()
