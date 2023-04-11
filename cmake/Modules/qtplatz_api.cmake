
function( qtplatz_adplugin_output_dir varName provider )

  if ( APPLE )
    # QTC_BINARY_DIR  := bin
    # IDE_PLUGIN_PATH := qtplatz.app/Contents/PlugIns
    set( ${varName} "${QTC_BINARY_DIR}/${IDE_PLUGIN_PATH}/${provider}" PARENT_SCOPE) # := QTPLATZ_PLUGIN_DIRECTORY
  else()
    # QTC_BINARY_DIR  := qtplatz.app
    # IDE_PLUGIN_PATH := lib/qtcreator/plugins
    set( ${varName} "${QTC_BINARY_DIR}/${IDE_PLUGIN_PATH}/${provider}" PARENT_SCOPE) # := QTPLATZ_PLUGIN_DIRECTORY
  endif()

endfunction()
