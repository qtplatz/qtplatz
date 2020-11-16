cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0022 NEW )
cmake_policy( SET CMP0028 NEW )

project( aqmd3 )

set( DISABLE_IVI_COM TRUE )

add_definitions( -DAQMD3_LIBRARY )


if ( WIN32 )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS -wd4996 )
  set( IVIROOTDIR "C:/Program Files/IVI Foundation/IVI" )
  set( VXIPNPPATH "C:/Program Files/IVI Foundation/VISA" )
endif()

if ( WIN32 AND NOT DISABLE_IVI_COM )

  list( APPEND aqmd3_sources digitizer_windows.cpp )

else()

  list( APPEND aqmd3_sources digitizer_linux.cpp aqmd3.cpp )
  list( APPEND aqmd3_sources aqmd3.hpp )

endif()

if ( ${CMAKE_SYSTEM_NAME} MATCHES Linux )
  set( LIBRT rt )
endif()

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/contrib/acqiris/libs
  ${CMAKE_SOURCE_DIR}/contrib/linux
  ${AqMD3_INCLUDE_DIR}
  )

if ( WIN32 )
  include_directories(
    ${IVIROOTDIR}/Bin
    ${VXIPNPPATH}/WinNT/include
    ${VXIPNPPATH}/WinNT/agvisa/include
    ${VXIPNPPATH}/VisaCom64
    )
endif()

add_library( ${PROJECT_NAME} SHARED
  aqmd3_global.hpp
  ${aqmd3_sources}
  aqmd3controller.cpp
  configfile.cpp
  configfile.hpp
  digitizer.hpp
  ppio.cpp
  ppio.hpp
  session.cpp
  session.hpp
  singleton.cpp
  singleton.hpp
  simulator.cpp
  simulator.hpp
  waveformobserver.cpp
  waveformobserver.hpp
)

target_link_libraries( ${PROJECT_NAME} PUBLIC
  adlog
  adfs
  adutils
  adportable
  adcontrols
  acqrscontrols
  dgpio
  aqmd3controls
  ${AqMD3_LIBRARIES}
  ${Boost_LIBRARIES}
  ${LIBRT}
  )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
