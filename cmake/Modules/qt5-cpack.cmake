
#message( STATUS "##### qt5-cpack.cmake<qtplatz> #####" )

include( "soname" )

find_package( Qt${QT_VERSION_MAJOR}
  REQUIRED
  Core
  DBus
  Gui
  Multimedia
#  MultimediaWidgets
  Network
  OpenGL
  PrintSupport
#  Sensors
  Sql
  Svg
#  Positioning
  Widgets
  Xml
#  XmlPatterns
  )

get_target_property( _loc Qt${QT_VERSION_MAJOR}::Core LOCATION )
if ( NOT _loc )
  message( FATAL "##### Qt${QT_VERSION_MAJOR}::Core LOCATION " ${_loc} " cannot be found")
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

foreach( lib
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Multimedia
#    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::PrintSupport
#    Qt${QT_VERSION_MAJOR}::Sensors
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Svg
#    Qt${QT_VERSION_MAJOR}::Positioning
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Xml
    #    Qt${QT_VERSION_MAJOR}::XmlPatterns
    )

  get_target_property( _loc ${lib} LOCATION )

  message( STATUS "## qt5-cpack install: " ${lib} " --> " ${_loc} --> ${QTPLATZ_QT5_RUNTIME_INSTALL_DIRECTORY} )

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
