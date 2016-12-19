# find RDKit

if ( rdkit_FOUND )
  return()
endif()

if ( WIN32 )

  find_package( Boost QUIET )
  
  set( _rdkit_libdirs
    "${CMAKE_SOURCE_DIR}/../rdkit/lib"  # rdkit default install (intree)
    "C:/RDKit/lib"
    )

else()

  set( _rdkit_libdirs "${CMAKE_SOURCE_DIR}/../rdkit" "${RDBASE}/lib" "$ENV{RDBASE}/lib" "/usr/local/lib" )

endif()

set( _rdkit_incdirs
  "${CMAKE_SOURCE_DIR}/../rdkit/Code"
  "${CMAKE_SOURCE_DIR}/../rdkit/External"
  "${RDBASE}/Code"  
  "${RDBASE}/External"
  "/usr/local/include/rdkit" )

find_path( _include_dir GraphMol/RDKitBase.h HINTS ${_rdkit_incdirs} )

if ( NOT _include_dir )
  return()
endif()

find_path( _inchi_inc_dir INCHI-API/inchi.h HINTS ${_rdkit_incdirs} )

find_library( _fileparsers_lib NAMES FileParsers HINTS
  ${_rdkit_libdirs}
  ${_include_dir}/../lib
  /usr/local/lib
  /usr/lib )

message( "#####" )
message( "##### rdkit fileparsers_lib: " ${_fileparsers_lib} )

if ( _fileparsers_lib )
  get_filename_component ( _libdir ${_fileparsers_lib} PATH )
else()
  return()
endif()

# replace RDBASE
get_filename_component ( RDBASE ${_libdir} DIRECTORY )

set ( RDKit_LIBRARIES
  EigenSolvers
  Optimizer
  ForceField
  DistGeometry
  Catalogs
  GraphMol
  Depictor
  FileParsers
  SmilesParse
  SubstructMatch
  ChemReactions
  ChemTransforms
  Subgraphs
  FilterCatalog
  Descriptors
  Fingerprints
  PartialCharges
  MolTransforms
  ForceFieldHelpers
  DistGeomHelpers
  MolAlign
  MolChemicalFeatures
  ShapeHelpers
  MolCatalog
  MolDraw2D
  FMCS
  MolHash
  MMPA
  StructChecker
  ReducedGraphs
  Trajectory
  SLNParse
  SimDivPickers
  InfoTheory
  RDGeometryLib
  RDGeneral
  Inchi
  RDInchiLib
  )

set ( rdkit_FOUND TRUE )
set ( RDKit_INCLUDE_DIRS ${_include_dir} )
if ( _inchi_inc_dir )
  list( APPEND RDKit_INCLUDE_DIRS ${_inchi_inc_dir} )
endif()

set ( RDKit_LIBRARY_DIRS ${_libdir} )

find_path( _moldraw2d_include NAMES "MolDraw2DSVG.h" PATHS "${_include_dir}/GraphMol/MolDraw2D" )

if ( _moldraw2d_include )
  list ( APPEND RDKit_INCLUDE_DIRS ${_moldraw2d_include} )
endif()

foreach ( _name ${RDKit_LIBRARIES} )

  set( _lib "_lib-NOTFOUND" )
  find_library( _lib NAMES ${_name} HINTS ${_libdir} )

  if ( WIN32 )
    # static-link library
    add_library( ${_name} STATIC IMPORTED )    

    set( _lib_debug "_lib-NOTFOUND" )
    find_library( _lib_debug NAMES ${_name}d HINTS ${_libdir} )
    set_target_properties( ${_name} PROPERTIES
      IMPORTED_LOCATION ${_lib}
      IMPORTED_IMPLIB ${_lib}
      IMPORTED_IMPLIB_DEBUG ${_lib_debug} )
  else()
    # shared library
    add_library( ${_name} SHARED IMPORTED )
    set_target_properties( ${_name} PROPERTIES IMPORTED_LOCATION ${_lib} )
  endif()

  get_target_property( value ${_name} IMPORTED_IMPLIB )
  message( STATUS "target :  " ${_name} "\tIMPOTED_IMPLIB: " ${value} )
  get_target_property( value ${_name} IMPORTED_IMPLIB_DEBUG )
  message( STATUS "target :  " ${_name} "\tIMPOTED_IMPLIB_DEBUG: " ${value} )

endforeach()

find_file( version_cmake NAMES rdkit-config-version.cmake PATHS ${RDKit_LIBRARY_DIRS} NO_DEFAULT_PATH )

if ( version_cmake )
  include( ${version_cmake} )
  set( RDKit_PACKAGE_VERSION ${PACKAGE_VERSION} )
endif()



