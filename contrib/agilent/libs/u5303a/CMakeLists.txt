cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0022 NEW )
cmake_policy( SET CMP0028 NEW )

project( u5303a )

set( DISABLE_IVI_COM TRUE )

add_definitions( -DU5303A_LIBRARY )

if ( WIN32 )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS -wd4996 )
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

if ( ${CMAKE_SYSTEM_NAME} MATCHES Linux )
  set( LIBRT rt )
endif()

add_library( u5303a SHARED
  ${u5303a_SOURCES}
  ${u5303a_HEADERS}
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${CMAKE_SOURCE_DIR}/contrib/socfpga/libs
  ${CMAKE_SOURCE_DIR}/contrib/linux
  ${AgMD2_INCLUDE_DIR}
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
  dgpio
  ${AgMD2_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LIBRT}
  )

runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
  LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
  ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
  )
