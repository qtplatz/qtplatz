
function( qtplatz_adplugin_output_dir varName provider )

  if ( APPLE )
    set( ${varName} "${CMAKE_BINARY_DIR}/bin/qtplatz.app/Contents/PlugIns/${provider}" PARENT_SCOPE) # := QTPLATZ_PLUGIN_DIRECTORY
  else()
    if ( IDE_VERSION VERSION_GREATER_EQUAL "9.0.0" )
      set( ${varName} "${CMAKE_BINARY_DIR}/${QTC_BINARY_DIR}/${IDE_PLUGIN_PATH}/${provider}" PARENT_SCOPE) # := QTPLATZ_PLUGIN_DIRECTORY
    else()
      set( ${varName} "${QTPLATZ_PLUGIN_DIRECTORY}/${provider}" PARENT_SCOPE) # := lib/qtplatz/plugins
    endif()
  endif()

endfunction()
