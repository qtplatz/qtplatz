
#message( STATUS "##### qt5-cpack.cmake<qtplatz> #####" )

include( "soname" )

if ( ${QT_VERSION_MAJOR} GREATER_EQUAL 6 )
  set ( CORE5COMPAT Core5Compat )
  set ( SVGWIDGET   SvgWidgets )
endif()

find_package( Qt${QT_VERSION_MAJOR}
  REQUIRED
  Concurrent
  Core
  DBus
  Gui
  Multimedia
  Network
  OpenGL
  PrintSupport
  Qml
  Sql
  Svg
  Test
  Widgets
  Xml
  ${SVGWIDGET}
  ${CORE5COMPAT}
  )

get_target_property( _loc Qt${QT_VERSION_MAJOR}::Core LOCATION )
if ( NOT _loc )
  message( FATAL "##### Qt${QT_VERSION_MAJOR}::Core LOCATION " ${_loc} " cannot be found")
else()
  message( STATUS "##### Qt${QT_VERSION_MAJOR}::Core LOCATION " ${_loc} )
endif()

get_filename_component( _dir ${_loc} DIRECTORY )

foreach( _lib "icu*" Qt${QT_VERSION_MAJOR}CLucene Qt${QT_VERSION_MAJOR}XcbQpa )
  if ( WIN32 )
    file( GLOB files "${_dir}/${_lib}.${SO}" )
    install( PROGRAMS ${files} DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
  else()
    file( GLOB files "${_dir}/lib${_lib}.${SO}.*" )
    install( PROGRAMS ${files} DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
  endif()
endforeach()

list ( APPEND _qtlibs
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Concurrent
  Qt${QT_VERSION_MAJOR}::DBus
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Multimedia
  Qt${QT_VERSION_MAJOR}::Network
  Qt${QT_VERSION_MAJOR}::OpenGL
  Qt${QT_VERSION_MAJOR}::PrintSupport
  Qt${QT_VERSION_MAJOR}::Sql
  Qt${QT_VERSION_MAJOR}::Svg
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Xml
  Qt${QT_VERSION_MAJOR}::Test
  Qt${QT_VERSION_MAJOR}::Qml
  )

if ( ${QT_VERSION_MAJOR} GREATER_EQUAL 6 )
  list( APPEND _qtlibs Qt${QT_VERSION_MAJOR}::Core5Compat )
  list( APPEND _qtlibs Qt${QT_VERSION_MAJOR}::SvgWidgets )
endif()

foreach( lib  ${_qtlibs}  )

  get_target_property( _loc ${lib} LOCATION )

  message( STATUS "## qt5-cpack install: ${lib} --> ${_loc} --> ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY}" )

  if ( WIN32 )
    install( FILES ${_loc} DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
  else()
    get_filename_component( name ${_loc} NAME_WE )
    get_filename_component( path ${_loc} DIRECTORY )
    file( GLOB files "${path}/${name}.${SO}.*" )
    install( PROGRAMS ${files} DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
  endif()

endforeach()

if ( QMAKE )
  execute_process( COMMAND ${QMAKE} -query QT_INSTALL_PLUGINS
    OUTPUT_VARIABLE QT_INSTALL_PLUGINS ERROR_VARIABLE qterr OUTPUT_STRIP_TRAILING_WHITESPACE )
else()
  message( FATAL "qmake not found" )
endif()

if ( QT_INSTALL_PLUGINS )
  file( GLOB _plugins RELATIVE ${QT_INSTALL_PLUGINS} "${QT_INSTALL_PLUGINS}/*" )
  list( REMOVE_ITEM _plugins audio bearer canbus designer gamepads qml1tooling qmltooling xchglintegrations )
else()
  #message( FATAL_ERROR "plugins: " ${QT_INSTALL_PLUGINS} )
endif()

foreach( plugin ${_plugins} )
  #message( STATUS "## qt5-cpack install: " ${QT_INSTALL_PLUGINS}/${plugin} )
  install( DIRECTORY "${QT_INSTALL_PLUGINS}/${plugin}"
    USE_SOURCE_PERMISSIONS
    DESTINATION plugins
    COMPONENT plugins
    FILES_MATCHING
    PATTERN "*d.${SO}" EXCLUDE
    PATTERN "*.${SO}"
    )
endforeach()

file( WRITE ${CMAKE_BINARY_DIR}/qt.conf "[Paths]\nPrefix=..\n" )
install( FILES ${CMAKE_BINARY_DIR}/qt.conf DESTINATION bin COMPONENT runtime_libraries )

message( STATUS "###################### TODO #############################" )
message( STATUS "## need to add ld.so.conf.d/qtplatz.conf for Qt6Test.so #" )
message( STATUS "##" )
message( STATUS "## qtplatz plugins need to be installed under qtcreater #" )
message( STATUS "###################### END TODO #########################" )
