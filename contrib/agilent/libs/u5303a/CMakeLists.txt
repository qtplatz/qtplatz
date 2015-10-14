cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0022 NEW )
cmake_policy( SET CMP0028 NEW )

project( u5303a )

#include(../../agilentlibrary.pri)
#include(../../boost.pri)

find_package(Boost 1.57 REQUIRED COMPONENTS date_time regex system filesystem wserialization serialization)

set( IVIROOTDIR "C:/Program Files/IVI Foundation/IVI" )
set( VXIPNPPATH "C:/Program Files/IVI Foundation/VISA" )

add_definitions(-DU5303A_LIBRARY -D_SCL_SECURE_NO_WARNINGS)

set( u5303a_SOURCES
  digitizer.cpp
  simulator.cpp
  sampleprocessor.cpp
  u5303acontroller.cpp
  waveformobserver.cpp
  session.cpp
  )

set( u5303a_HEADERS
  u5303a_global.hpp
  digitizer.hpp
  safearray.hpp
  simulator.hpp
  sampleprocessor.hpp
  waveformobserver.hpp
  session.hpp
  )

add_library( u5303a SHARED
  ${u5303a_SOURCES}
  ${u5303a_HEADERS}
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${IVIROOTDIR}/Include
  ${IVIROOTDIR}/Bin
  ${VXIPNPPATH}/WinNT/include
  ${VXIPNPPATH}/WinNT/agvisa/include
  ${VXIPNPPATH}/VisaCom64
  )

target_link_libraries( u5303a PUBLIC
  adlog
  adfs
  adutils
  adportable
  adcontrols
  acqrscontrols
  ${Boost_LIBRARIES}
  )
