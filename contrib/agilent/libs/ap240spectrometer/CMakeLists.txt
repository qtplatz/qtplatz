cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0020 NEW )

project( ap240spectrometer )

add_definitions(-DAP240SPECTROMETER_LIBRARY)
if ( MSVC )
  add_definitions(-D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

add_library( ap240spectrometer SHARED
  ap240spectrometer.cpp
  massspectrometer.cpp 
  datainterpreter.cpp
  ap240spectrometer.hpp
  ap240spectrometer_global.hpp
  massspectrometer.hpp
  datainterpreter.hpp
  )

target_link_libraries( ap240spectrometer LINK_PUBLIC
  adinterface
  adplugin
  adportable
  adcontrols
  acewrapper
  )

