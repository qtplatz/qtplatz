# qtplatz.cmake

##########################################
############ boost setup #################
##########################################
set( Boost_NO_SYSTEM_PATHS ON )

# See 'libs/serialization/src/basic_archive.cpp library_version_type for archive version
set ( __boost_versions
  "boost-1_85"        # V20
  "boost-1_84"        # V20 <--
  "boost-1_83"        # V19
  "boost-1_79"        # V19
  "boost-1_78"        # V19
  "boost-1_75"        # V18 <-- 'libs/serialization/src/basic_archive.cpp library_version_type(18)
  )

if ( WIN32 )

  set ( __boost_dirs ${__boost_versions} )
  list( TRANSFORM __boost_dirs PREPEND "C:/Boost/include/" )

  find_path( _boost NAMES boost HINTS ${__boost_dirs} )

  set( BOOST_ROOT ${_boost} )
  set( BOOST_INCLUDEDIR ${_boost} )
  set( BOOST_LIBRARYDIR "C:/Boost/lib" )

  list( APPEND CMAKE_PREFIX_PATH "C:/Boost/lib/cmake" )

  set( Boost_NO_SYSTEM_PATHS ON )
  set( Boost_USE_STATIC_LIBS OFF )
  add_definitions( "-DBOOST_DLL_USE_STD_FS" )
  add_definitions( "-DBOOST_ALL_NO_LIB" ) # <-- disable boost auto linking
  # On windows, boost::archive templates are not possible to implment across shared object boundary
  # if ( Boost_USE_STATIC_LIBS )
  add_definitions(
    "-DBOOST_ATOMIC_DYN_LINK"
    "-DBOOST_CHRONO_DYN_LINK"
    "-DBOOST_JSON_DYN_LINK"
    "-DBOOST_LOG_DYN_LINK"
  )
  #else()
  #  add_definitions( -DBOOST_ALL_DYN_LINK )
  #endif()

else()
  ## Boost setup for mac/linux
  set( Boost_USE_STATIC_LIBS OFF )
  set( Boost_NO_SYSTEM_PATHS ON )

  set ( __boost_dirs ${__boost_versions} )
  list( TRANSFORM __boost_dirs PREPEND "/usr/local/" )

  find_path( _boost NAMES include/boost HINTS ${__boost_dirs} )

  if ( _boost )
    set( BOOST_ROOT ${_boost} )
  endif()

endif()

if ( NOT Boost_FOUND )
  find_package( Boost 1.75 REQUIRED COMPONENTS
    atomic
    chrono
    container
    date_time
    iostreams
    json
    program_options
    regex
    random
    serialization
    system
    thread
    timer
    wserialization
  )
endif()

remove_definitions( "-DBOOST_NO_AUTO_PTR" )
