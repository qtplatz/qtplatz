
if ( NOT WIN32 )
  return()
endif()

message( STATUS "########################################################" )
message( STATUS "############ OpenSSL CPACK -- OpenSSL ${OPENSSL_VERSION} ############" )

set( __libs OpenSSL::SSL OpenSSL::Crypto)
set( __dlls )

foreach( lib ${__libs} )
  message( STATUS "## OpenSSL lib: ${lib}" )
  get_target_property( loc ${lib} IMPORTED_LOCATION_RELEASE )
  if ( loc )
    #set( __bindir "${__path}/../bin" )
    #cmake_path( NORMALIZE __bindir PARENT_PATH __path )
    #message( STATUS "## OpenSSL install: " ${lib} " --> " ${loc} "; parent: " ${__path})
    execute_process( COMMAND powershell.exe
      "lib /list \"${loc}\" | Select-String -Pattern '.dll' | sort -unique"
      OUTPUT_VARIABLE _dll COMMAND_ECHO NONE)
    string( STRIP ${_dll} __dll )
    #message( STATUS "output --> ${__dll}" )

    cmake_path( GET loc PARENT_PATH __path )
    cmake_path( SET __dllpath NORMALIZE "${__path}/../bin/${__dll}" )
    message( STATUS "bindir --> ${__dllpath}" )
    list ( APPEND __dlls ${__dllpath} )

    install ( PROGRAMS ${__dllpath} DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
  endif()

endforeach()
