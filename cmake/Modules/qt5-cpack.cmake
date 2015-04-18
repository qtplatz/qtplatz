
if ( Qt5_FOUND )

  find_package( Qt5 REQUIRED Core Gui Help Multimedia Network OpenGL PrintSupport Script Sql Svg Widgets Xml XmlPatterns )

  foreach(plugin ${Qt5Gui_PLUGINS} ${Qt5Svg_PLUGINS} ${Qt5Sql_PLUGINS} )
    get_target_property( _loc ${plugin} LOCATION )
    file( RELATIVE_PATH _rname $ENV{QTDIR}/plugins ${_loc} )
    get_filename_component(_rpath ${_rname} DIRECTORY )
    install( FILES ${_loc} DESTINATION plugins/${_rpath} COMPONENT runtime_libraries )
  endforeach()

  if ( WIN32 )
    set( SO "dll")
  elseif( APPLE )
    set( SO "dylib" )
  else()
    set( SO "so" )
  endif()

  get_target_property( _loc Qt5::Core LOCATION )
  get_filename_component( _dir ${_loc} DIRECTORY )

  # message( "qt4 dir: " ${_dir}/icuin52.${SO} )
  install( FILES
    ${_dir}/icudt53.${SO}
    ${_dir}/icuin53.${SO}
    ${_dir}/icuuc53.${SO}
    ${_dir}/Qt5CLucene.${SO}
    DESTINATION bin COMPONENT runtime_libraries )
  
  foreach( lib
      Qt5::Core
      Qt5::Gui
      Qt5::Help
      Qt5::Multimedia
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
    # message( "##### Qt5 : " ${_loc} )
    install( FILES ${_loc} DESTINATION bin COMPONENT runtime_libraries )

  endforeach()
endif()