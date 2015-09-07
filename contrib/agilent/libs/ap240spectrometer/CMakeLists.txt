cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0020 NEW )

project( ap240spectrometer )
set( PROVIDER MS-Cheminformatics )

add_definitions(-DAP240SPECTROMETER_LIBRARY)

if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.adplugin"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.adplugin"
  )

add_library( ap240spectrometer SHARED
  ap240spectrometer.cpp
  massspectrometer.cpp 
  datainterpreter.cpp
  ap240spectrometer.hpp
  ap240spectrometer_global.hpp
  massspectrometer.hpp
  datainterpreter.hpp
  waveform.hpp
  waveform.cpp
  threshold_method.hpp
  threshold_method.cpp
  method.hpp
  method.cpp
  )

target_link_libraries( ap240spectrometer LINK_PUBLIC
  adinterface
  adplugin
  adportable
  adcontrols
  acewrapper
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  LIBRARY DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

install( FILES
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.adplugin"  
  DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )
