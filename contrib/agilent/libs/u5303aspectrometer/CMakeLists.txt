cmake_policy( SET CMP0028 NEW )
cmake_policy( SET CMP0020 NEW )

project( u5303aspectrometer )

if ( MSVC )
  add_definitions(-DU5303ASPECTROMETER_LIBRARY -D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

#include(../../agilentlibrary.pri)
#include(../../boost.pri)

add_library( u5303aspectrometer SHARED
  u5303aspectrometer.cpp
  massspectrometer.cpp 
  datainterpreter.cpp
  u5303aspectrometer.hpp
  u5303aspectrometer_global.hpp
  massspectrometer.hpp
  datainterpreter.hpp
  )

target_link_libraries( u5303aspectrometer LINK_PUBLIC
  adplugin
  adportable
  adcontrols
  acewrapper
  )

