
#message( STATUS "##### qt5-cpack.cmake<qtplatz> #####" )

include( "soname" )

find_package( Qt5
  REQUIRED
  Core DBus Gui
  Multimedia
  MultimediaWidgets
  Network OpenGL PrintSupport Script Sensors Sql Svg
  Positioning Quick Qml WebChannel
  Widgets
  Xml XmlPatterns )

if ( "${Qt5_VERSION}" VERSION_LESS "5.6" )
  find_package( Qt5 REQUIRED WebKit WebKitWidgets )
  set( _webkits Qt5::WebKit Qt5::WebKitWidgets )
else()
  find_package( Qt5 REQUIRED WebView )
  set( _webkits Qt5::WebView )
endif()

message( STATUS "Qt5 Version: " ${Qt5_VERSION} "\t" ${Qt5_VERSION_MAJOR}.${Qt5_VERSION_MINOR})

get_target_property( _loc Qt5::Core LOCATION )
get_filename_component( _dir ${_loc} DIRECTORY )

foreach( _lib "icu*" Qt5CLucene Qt5XcbQpa )
  if ( WIN32 )
    file( GLOB files "${_dir}/${_lib}.${SO}" )
    install( PROGRAMS ${files} DESTINATION ${dest} COMPONENT runtime_libraries )  
  else()
    #file( GLOB files "${_dir}/libicu*.${SO}.*" "${_dir}/libQt5CLucene.${SO}.*" )
    file( GLOB files "${_dir}/lib${_lib}.${SO}.*" )
    install( PROGRAMS ${files} DESTINATION ${dest} COMPONENT runtime_libraries )
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
    Qt5::Script
    Qt5::Sensors
    Qt5::Sql
    Qt5::Svg
    Qt5::Positioning
    Qt5::Quick
    Qt5::Qml
    Qt5::WebChannel
    Qt5::Widgets
    ${_webkits}
    Qt5::Xml
    Qt5::XmlPatterns )
  
  get_target_property( _loc ${lib} LOCATION )
  #message( STATUS "## qt5-cpack install: " ${lib} " --> " ${_loc} )
  
  if ( WIN32 )
    install( FILES ${_loc} DESTINATION ${dest} COMPONENT runtime_libraries )
  else()
    get_filename_component( name ${_loc} NAME_WE )
    get_filename_component( path ${_loc} DIRECTORY )
    file( GLOB files "${path}/${name}.${SO}.*" )
    install( PROGRAMS ${files} DESTINATION ${dest} COMPONENT runtime_libraries )
  endif()
  
endforeach()

if ( QT_INSTALL_PLUGINS )
  file( GLOB _plugins RELATIVE ${QT_INSTALL_PLUGINS} "${QT_INSTALL_PLUGINS}/*" )
  list( REMOVE_ITEM _plugins audio bearer designer qml1tooling qmltooling xchglintegrations )
else()
  message( FATAL_ERROR "plugins: " ${QT_INSTALL_PLUGINS} )
endif()

message( STATUS "##### plugins: " ${QT_INSTALL_PLUGINS} )
foreach( plugin ${_plugins} )
  install( DIRECTORY "${QT_INSTALL_PLUGINS}/${plugin}" USE_SOURCE_PERMISSIONS DESTINATION plugins COMPONENT plugins )
endforeach()

file( WRITE ${CMAKE_BINARY_DIR}/qt.conf "[Paths]\nPrefix=..\n" )
install( FILES ${CMAKE_BINARY_DIR}/qt.conf DESTINATION bin COMPONENT runtime_libraries )
