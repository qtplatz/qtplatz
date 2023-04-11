
function( qtplatz_adplugin_output_dir varName provider )
  if ( APPLE )
    set( ${varName} "bin/${IDE_PLUGIN_PATH}/${provider}" PARENT_SCOPE) # := QTPLATZ
  else()
    set( ${varName} "${QTC_BINARY_DIR}/${IDE_PLUGIN_PATH}" PARENT_SCOPE) # 'qtplatz'/'lib/qtcreator/plugins ; /'provider
  endif()
endfunction()

function( qtplatz_adplugin_install_dir varName provider )
  if ( APPLE )
    set( ${varName} "bin/${IDE_PLUGIN_PATH}/${provider}" PARENT_SCOPE)
  else()
    set( ${varName} "${IDE_PLUGIN_PATH}" PARENT_SCOPE)     # lib/qtcreator/plugins/ ; + provider
  endif()
endfunction()

################## plugin output ##############
function( qtplatz_plugin_output_dir varName provider )
  if ( APPLE )
    set( ${varName} "bin/${IDE_PLUGIN_PATH}/${provider}" PARENT_SCOPE)
  else()
    set( ${varName} "${QTC_BINARY_DIR}/${IDE_PLUGIN_PATH}" PARENT_SCOPE)
  endif()
endfunction()

################## plugin install ##############
function( qtplatz_plugin_install_dir varName provider )
  if ( APPLE )
    set( ${varName} "bin/${IDE_PLUGIN_PATH}/${provider}" PARENT_SCOPE)
  else()
    set( ${varName} "${IDE_PLUGIN_PATH}" PARENT_SCOPE)
  endif()
endfunction()

################## install runpath ##############
function( qtplatz_install_rpath varName install_dir )
  if ( APPLE )
    set( ${varName} )
  else()
    set( ${varName} "$ORIGIN:$ORIGIN/../:$ORIGIN/../../:$ORIGIN/../lib:\$ORIGIN/../../qtplatz/:${CMAKE_INSTALL_RPATH}" PARENT_SCOPE)
  endif()
endfunction()

################## runtime_install_path ##############
function( runtime_install_path varName library )
  set( ${varName} "bin" PARENT_SCOPE)
endfunction()

################## library_install_path ##############
function( library_install_path varName library )
  if ( APPLE )
    set( ${varName} "lib/qtplatz" PARENT_SCOPE)
  else()
    set( ${varName} "lib"  PARENT_SCOPE)
  endif()
endfunction()

################## library_install_path ##############
function( archive_install_path varName library )
  set( ${varName} "lib" PARENT_SCOPE)
endfunction()
