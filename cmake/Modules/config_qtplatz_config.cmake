function( config_qtplatz_config installed )

  if ( installed )
    set ( QTPLATZ_INCLUDE_DIRS "\$\{_dir\}/include" )
    set ( QTPLATZ_BINARY_DIR   "\$\{_dir\}" )
    set ( QTPLATZ_LIBRARY_DIRS "\$\{_dir\}/lib" )
    set ( QTPLATZ_ARCHIVE_OUTPUT_DIRECTORY "\$\{_dir\}/lib" ) # .lib
    set ( QTPLATZ_LIBRARY_OUTPUT_DIRECTORY "\$\{_dir\}/lib/qtplatz" ) # module (dll)
    set ( QTPLATZ_RUNTIME_OUTPUT_DIRECTORY "\$\{_dir\}/bin" ) # exe
    set ( QTPLATZ_PLUGIN_DIRECTORY   "\$\{_dir}/lib/qtplatz/plugins" )
    
    configure_file(
      ${CMAKE_SOURCE_DIR}/qtplatz-config.cmake.in
      ${CMAKE_BINARY_DIR}/qtplatz-config_installed.cmake @ONLY )
  else()

    set ( QTPLATZ_INCLUDE_DIRS
      "${CMAKE_SOURCE_DIR}/src/libs"
      "${CMAKE_BINARY_DIR}/src/libs"
      "${CMAKE_SOURCE_DIR}/src" )
    set ( QTPLATZ_BINARY_DIR   "${CMAKE_BINARY_DIR}" )
    #set ( QTPLATZ_LIBRARY_DIRS "${CMAKE_BINARY_DIR}/lib" )
    set ( QTPLATZ_LIBRARY_DIRS "\$\{_dir\}/lib" )
    
    set ( QTPLATZ_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}" ) # .lib
    set ( QTPLATZ_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" ) # module (dll)
    set ( QTPLATZ_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" ) # exe
    set ( QTPLATZ_PLUGIN_DIRECTORY   "${QTPLATZ_PLUGIN_DIRECTORY}/lib/qtplatz/plugins" )

    configure_file(
      ${CMAKE_SOURCE_DIR}/qtplatz-config.cmake.in
      ${CMAKE_BINARY_DIR}/qtplatz-config.cmake @ONLY )
  endif()
endfunction()
