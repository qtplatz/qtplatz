cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0020 NEW )

project( acqrscontrols )
set( PROVIDER MS-Cheminformatics )

add_definitions(-DACQRSCONTROLS_LIBRARY)

if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${CMAKE_SOURCE_DIR}/src/libs
  )

add_library( ${PROJECT_NAME} SHARED
  acqrscontrols_global.hpp
  resultwriter.hpp
  u5303a/averagedata.hpp
  u5303a/averagedata.cpp
  u5303a/device_method.hpp
  u5303a/device_method.cpp
  u5303a/identify.hpp
  u5303a/identify.cpp
  u5303a/metadata.hpp
  u5303a/metadata.cpp
  u5303a/method.hpp
  u5303a/method.cpp
  u5303a/tdcdoc.cpp
  u5303a/tdcdoc.hpp
  u5303a/threshold_result.hpp
  u5303a/threshold_result.cpp
  u5303a/histogram.hpp
  u5303a/histogram.cpp
  u5303a/waveform.hpp
  u5303a/waveform.cpp
  ap240/averagedata.hpp
  ap240/averagedata.cpp
  ap240/datainterpreter.cpp
  ap240/datainterpreter.hpp
  ap240/histogram.cpp
  ap240/histogram.hpp
  ap240/metadata.cpp
  ap240/metadata.hpp
  ap240/method.cpp
  ap240/method.hpp
  ap240/tdcdoc.cpp
  ap240/tdcdoc.hpp  
  ap240/threshold_result.cpp
  ap240/threshold_result.hpp
  ap240/threshold_result_accessor.cpp
  ap240/threshold_result_accessor.hpp
  ap240/waveform.cpp
  ap240/waveform.hpp
  ap240/waveform_accessor.cpp
  ap240/waveform_accessor.hpp
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adicontroller
  adplugin
  adportable
  adcontrols
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT runtime_libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
