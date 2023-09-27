# qtplatz.cmake

##########################################
############ boost setup #################
##########################################
set( Boost_NO_SYSTEM_PATHS ON )

# See 'libs/serialization/src/basic_archive.cpp library_version_type for archive version
set ( __boost_versions
#  "boost-1_83"        # V19 <-- json/value.hpp cannot be compiled w/ clang-15
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
  set( Boost_USE_STATIC_LIBS ON )
  #add_definitions( -DBOOST_ALL_NO_LIB ) # <-- disable boost auto linking
  # On windows, boost::archive templates are not possible to implment across shared object boundary
  if ( Boost_USE_STATIC_LIBS )
    add_definitions(
      -DBOOST_LOG_DYN_LINK
#      -DBOOST_ATOMIC_DYN_LINK
#      -DBOOST_BZIP2_DYN_LINK
#      -DBOOST_CHRONO_DYN_LINK
#      -DBOOST_RANDOM_DYN_LINK
#      -DBOOST_SYSTEM_DYN_LINK
#      -DBOOST_TIMER_DYN_LINK
#      -DBOOST_FILESYSTEM_DYN_LINK
#      -DBOOST_JSON_DYN_LINK
    )
  else()
    add_definitions( -DBOOST_ALL_DYN_LINK )
  endif()

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

remove_definitions( "-DBOOST_NO_AUTO_PTR" )
