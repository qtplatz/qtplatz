
find_package( Boost REQUIRED )

include( "soname" )

if ( Boost_FOUND )

  set ( libs bzip2 chrono date_time filesystem iostreams program_options regex serialization system thread timer wserialization )

  foreach ( lib ${libs} )
    file( GLOB _libs ${Boost_LIBRARY_DIR}/libboost_${lib}.${SO}* )
    install( PROGRAMS ${_libs} DESTINATION ${dest} COMPONENT runtime_libraries )
  endforeach()

endif()