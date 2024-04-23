
function( qtplatz_adplugin_output_dir varName provider )
  if ( APPLE )
    # set( ${varName} "bin/${IDE_PLUGIN_PATH}/${provider}" PARENT_SCOPE)
    set( ${varName} "bin/qtplatz.app/Contents/PlugIns/${provider}" PARENT_SCOPE)
  else()
    set( ${varName} "${QTC_BINARY_DIR}/lib/qtplatz/plugins" PARENT_SCOPE)
  endif()
endfunction()

function( qtplatz_adplugin_install_dir varName provider )
  if ( APPLE )
    # set( ${varName} "bin/${IDE_PLUGIN_PATH}/${provider}" PARENT_SCOPE)
    set( ${varName} "bin/qtplatz.app/Contents/PlugIns/${provider}" PARENT_SCOPE)
  else()
    set( ${varName} "lib/qtplatz/plugins" PARENT_SCOPE)
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

################## runtime_install_path ##############
function( runtime_install_path varName library )
  set( ${varName} "bin" PARENT_SCOPE)
endfunction()

################## library_install_path (.so .dylib files) ##############
function( library_install_path varName library )
  if ( APPLE )
    set( ${varName} "Contents/Frameworks" PARENT_SCOPE )
  else()
    set( ${varName} "lib"  PARENT_SCOPE ) # <- formerly lib/qtplatz
  endif()
endfunction()

################## library_install_path ##############
function( archive_install_path varName library )
  set( ${varName} "lib" PARENT_SCOPE)
endfunction()

################## install runpath ##############
function( qtplatz_install_rpath varName library install_dir )
  if ( APPLE )
    set( ${varName} PARENT_SCOPE )
  else()
    if ( "${install_dir}" STREQUAL "${IDE_PLUGIN_PATH}" )
      set ( ${varName} "\${ORIGIN}:\${ORIGIN}/../../qtplatz:\${ORIGIN}/../.." PARENT_SCOPE )
    else()
      set( ${varName} "${CMAKE_INSTALL_RPATH}" PARENT_SCOPE )
    endif()
  endif()
endfunction()
