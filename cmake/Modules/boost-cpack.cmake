
if ( NOT Boost_FOUND )
  find_package( Boost REQUIRED )
endif()

include( "soname" )

if ( WITH_PYTHON3 AND ( NOT CMAKE_CROSSCOMPILING AND NOT RTC_ARCH_ARM ) )
  find_package( Python3 COMPONENTS Interpreter Development )
  if ( Python3_FOUND )
    set( python3 "python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}" )
  endif()
endif()

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
    random
    serialization
    system
    thread
    timer
    wserialization
    ${python3}
    )

  if ( WIN32 )
    foreach ( lib ${libs} )
      file( GLOB files ${Boost_LIBRARY_DIRS}/boost_${lib}-mt-x64-1_*.dll )
      install( PROGRAMS ${files} DESTINATION ${QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
    endforeach()
  else()
    foreach ( lib ${libs} )
      file( GLOB files ${Boost_LIBRARY_DIRS}/libboost_${lib}.${SO}* )
      install( PROGRAMS ${files} DESTINATION ${QTPLATZ_COMMON_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
    endforeach()
  endif()

endif()
