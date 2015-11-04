cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0022 NEW )
cmake_policy( SET CMP0028 NEW )

project( u5303a )

set( DISABLE_IVI_COM TRUE )

find_package(Boost 1.57 REQUIRED COMPONENTS date_time regex system filesystem wserialization serialization)

add_definitions( -DU5303A_LIBRARY )

if ( WIN32 )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS )
  set( IVIROOTDIR "C:/Program Files/IVI Foundation/IVI" )
  set( VXIPNPPATH "C:/Program Files/IVI Foundation/VISA" )
endif()

set( u5303a_SOURCES
  simulator.cpp
  u5303acontroller.cpp
  waveformobserver.cpp
  session.cpp
  ppio.cpp
  )

if ( WIN32 AND NOT DISABLE_IVI_COM )

  list( APPEND u5303a_SOURCES digitizer_windows.cpp )

else()
  
  list( APPEND u5303a_SOURCES digitizer_linux.cpp agmd2.cpp )
  list( APPEND u5303a_HEADERS agmd2.hpp )

  if ( WIN32 )
    find_library( AgMD2_LIBRARY AgMD2 ${IVIROOTDIR}/Lib_x64/msc )
  else()
    set( AgMD2_LIBRARY "AgMD2" )
  endif()

endif()

set( u5303a_HEADERS
  u5303a_global.hpp
  digitizer.hpp
  safearray.hpp
  simulator.hpp
  waveformobserver.hpp
  session.hpp
  ppio.hpp
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
  ${AgMD2_LIBRARY}
  ${Boost_LIBRARIES}
  )
