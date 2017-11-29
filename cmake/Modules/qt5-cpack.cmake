
#message( STATUS "##### qt5-cpack.cmake<qtplatz> #####" )

include( "soname" )

find_package( Qt5
  REQUIRED
  Core
  DBus
  Gui
  Multimedia
  MultimediaWidgets
  Network
  OpenGL
  PrintSupport
  Sensors
  Sql
  Svg
  Positioning
  Widgets
  Xml
  XmlPatterns
  )

if ( "${Qt5_VERSION}" VERSION_LESS "5.6" )
  find_package( Qt5 REQUIRED WebKit WebKitWidgets WebEngineWidgets )
  set( _webkits Qt5::WebKit Qt5::WebKitWidgets )
endif()

get_target_property( _loc Qt5::Core LOCATION )
get_filename_component( _dir ${_loc} DIRECTORY )

foreach( _lib "icu*" Qt5CLucene Qt5XcbQpa )
  if ( WIN32 )
    file( GLOB files "${_dir}/${_lib}.${SO}" )
    install( PROGRAMS ${files} DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )  
  else()
    file( GLOB files "${_dir}/lib${_lib}.${SO}.*" )
    install( PROGRAMS ${files} DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
  endif()
endforeach()

foreach( lib
    Qt5::Core
    Qt5::DBus
    Qt5::Gui
    Qt5::Multimedia
    Qt5::MultimediaWidgets
    Qt5::Network
    Qt5::OpenGL
    Qt5::PrintSupport
    Qt5::Sensors
    Qt5::Sql
    Qt5::Svg
    Qt5::Positioning
    Qt5::Widgets
    ${_webkits}
    Qt5::Xml
    Qt5::XmlPatterns )
  
  get_target_property( _loc ${lib} LOCATION )

  # message( STATUS "## qt5-cpack install: " ${lib} " --> " ${_loc} --> ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} )
  
  if ( WIN32 )
    install( FILES ${_loc} DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
  else()
    get_filename_component( name ${_loc} NAME_WE )
    get_filename_component( path ${_loc} DIRECTORY )
    file( GLOB files "${path}/${name}.${SO}.*" )
    install( PROGRAMS ${files} DESTINATION ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} COMPONENT runtime_libraries )
  endif()
  
endforeach()

if ( QT_INSTALL_PLUGINS )
  file( GLOB _plugins RELATIVE ${QT_INSTALL_PLUGINS} "${QT_INSTALL_PLUGINS}/*" )
  list( REMOVE_ITEM _plugins audio bearer canbus designer gamepads qml1tooling qmltooling xchglintegrations )
else()
  message( FATAL_ERROR "plugins: " ${QT_INSTALL_PLUGINS} )
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
