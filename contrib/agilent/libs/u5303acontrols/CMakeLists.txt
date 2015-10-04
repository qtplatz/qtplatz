cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0020 NEW )

project( u5303acontrols )
set( PROVIDER MS-Cheminformatics )

add_definitions(-DU5303ACONTROLS_LIBRARY)

if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

add_library( ${PROJECT_NAME} SHARED
  u5303acontrols_global.hpp
  device_method.hpp
  device_method.cpp
  identify.hpp
  identify.cpp
  metadata.hpp
  metadata.cpp
  method.hpp
  method.cpp
  threshold_result.hpp
  threshold_result.cpp
  histogram.hpp
  histogram.cpp
  waveform.hpp
  waveform.cpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adicontroller
  adplugin
  adportable
  adcontrols
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
