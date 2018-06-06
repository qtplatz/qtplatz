
if ( NOT Boost_FOUND )
  find_package( Boost REQUIRED )
endif()

include( "soname" )

if ( Boost_FOUND )

  set ( libs
    atomic
    bzip2
    chrono
    date_time
    filesystem
    iostreams
    locale
    log
    log_setup
    program_options
    regex
    serialization
    system
    thread
    timer
    wserialization )

  foreach ( lib ${libs} )
    file( GLOB files ${Boost_LIBRARY_DIRS}/libboost_${lib}.${SO}* )
    install( PROGRAMS ${files} DESTINATION ${QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
  endforeach()

endif()
