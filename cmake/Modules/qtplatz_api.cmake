
function( qtplatz_adplugin_output_dir varName provider )

  if ( APPLE )
    set( ${varName} "bin/${IDE_PLUGIN_PATH}/${provider}" PARENT_SCOPE) # := QTPLATZ
  else()
    set( ${varName} "${QTC_BINARY_DIR}/${IDE_PLUGIN_PATH}/${provider}" PARENT_SCOPE) # 'qtplatz'/'lib/qtcreator/plugins/'provider
  endif()

endfunction()

function( qtplatz_adplugin_install_dir varName provider )

  if ( APPLE )
    set( ${varName} "bin/${IDE_PLUGIN_PATH}/${provider}" PARENT_SCOPE)
  else()
    set( ${varName} "${IDE_PLUGIN_PATH}/${provider}" PARENT_SCOPE)     # lib/qtcreator/plugins/ + provider
  endif()

endfunction()
