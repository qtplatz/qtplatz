
message( STATUS "##### qt5-cpack.cmake<qtplatz> #####" )

include( "soname" )

find_package( Qt5 REQUIRED Core Gui Network OpenGL PrintSupport Script Sql Svg Widgets Xml XmlPatterns )

get_target_property( _loc Qt5::Core LOCATION )
get_filename_component( _dir ${_loc} DIRECTORY )

if ( WIN32 )
  file( GLOB files "${_dir}/icu*.${SO}" "${_dir}/Qt5CLucene.${SO}" )
  install( PROGRAMS ${files} DESTINATION ${dest} COMPONENT runtime_libraries )  
else()
  file( GLOB files "${_dir}/libicu*.${SO}.*" "${_dir}/libQt5CLucene.${SO}.*" )
  install( PROGRAMS ${files} DESTINATION ${dest} COMPONENT runtime_libraries )
endif()

foreach( lib
    Qt5::Core
    Qt5::Gui
    Qt5::Network
    Qt5::OpenGL
    Qt5::PrintSupport
    Qt5::Script
    Qt5::Sql
    Qt5::Svg
    Qt5::Widgets
    Qt5::Xml
    Qt5::XmlPatterns )
  
  get_target_property( _loc ${lib} LOCATION )
  message( STATUS "## qt5-cpack install " ${_loc} " --> runtime_libraries" )
  
  if ( WIN32 )
    install( FILES ${_loc} DESTINATION ${dest} COMPONENT runtime_libraries )
  else()
    get_filename_component( name ${_loc} NAME_WE )
    get_filename_component( path ${_loc} DIRECTORY )
    file( GLOB files "${path}/${name}.${SO}.*" )
    install( PROGRAMS ${files} DESTINATION ${dest} COMPONENT runtime_libraries )
  endif()
  
endforeach()

find_path( plugins_dir NAMES platforms PATHS ${QTDIR}/plugins ${_dir}/qt5/plugins )

if ( plugins_dir )
  file( GLOB _plugins RELATIVE ${plugins_dir} "${plugins_dir}/*" )
  list( REMOVE_ITEM _plugins audio bearer designer qml1tooling qmltooling xchglintegrations )
else()
  message( FATAL_ERROR "plugins: " ${plugins_dir} )
endif()

foreach( plugin ${_plugins} )
  install( DIRECTORY ${plugins_dir}/${plugin} USE_SOURCE_PERMISSIONS DESTINATION plugins )
endforeach()

#find_path( qt_conf NAMES qt.conf PATHS ${QTDIR}/bin ${plugin_dir} ${plugin_dir}/bin )
#if ( qt_conf )
#  install( FILES ${qt_conf}/qt.conf DESTINATION bin )
#endif()
