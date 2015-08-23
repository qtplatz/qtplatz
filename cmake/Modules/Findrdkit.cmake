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
else()

  find_package( rdkit CONFIG HINTS
    ${CMAKE_SOURCE_DIR}/../rdkit/lib
    ${CMAKE_BINARY_DIR}/../rdkit
    $ENV{RDBASE}
    $ENV{RDBASE}/build
    /usr/local/lib
    )

endif()

find_path( _include_dir GraphMol/RDKitBase.h HINTS
  $ENV{RDBASE}/Code
  ${CMAKE_SOURCE_DIR}/../rdkit/Code
  /usr/local/include/rdkit
  /usr/include/rdkit
  )

if ( _include_dir )
  get_filename_component ( rdbase ${_include_dir} PATH )
  if ( ${rdbase} MATCHES "/usr/include" )
    set( rdbase "/usr" )
  endif()
  if ( ${rdbase} MATCHES "/usr/local/include" )
    set( rdbase "/usr/local" )
  endif()
else()
  message( STATUS "Findrdkit: rdkit not found" )
  return()
endif()

find_library( _fileparsers_lib NAMES FileParsers HINTS
  ${_include_dir}/../lib
  /usr/local/lib
  /usr/lib )

if ( _fileparsers_lib )
  get_filename_component ( _libdir ${_fileparsers_lib} PATH )
endif()

if ( _include_dir AND _libdir )
  
  set ( rdkit_FOUND TRUE )
  set ( RDKit_INCLUDE_DIRS ${_include_dir} )
  set ( RDKit_LIBRARY_DIRS ${_libdir} )

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

  find_file( version_cmake NAMES rdkit-config-version.cmake PATHS ${_libdir} NO_DEFAULT_PATH )

  if ( version_cmake )
    include( ${version_cmake} )
    set( RDKit_PACKAGE_VERSION ${PACKAGE_VERSION} )
  endif()
  
endif()

