cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0022 NEW )
cmake_policy( SET CMP0028 NEW )

project( ap240 )

find_package(Boost 1.57 REQUIRED COMPONENTS date_time regex system filesystem wserialization serialization)

add_definitions(-DAP240_LIBRARY -D_SCL_SECURE_NO_WARNINGS)

set( platform_libs "" )

if ( ${CMAKE_SYSTEM} MATCHES "Linux" )
  set( platform_libs dl )
  add_definitions( -D_LINUX -D_ACQIRIS )
endif()

set( ap240_SOURCES
  digitizer.cpp
  simulator.cpp
  sampleprocessor.cpp
  )

set( ap240_HEADERS
  ap240_global.hpp
  digitizer.hpp
  safearray.hpp
  simulator.hpp
  sampleprocessor.hpp
  )

add_library( ap240 SHARED
  ${ap240_SOURCES}
  ${ap240_HEADERS}
  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  )

if ( WIN32 )
  include_directories( "C:/Program Files (x86)/Agilent/Acqiris/include" )
  link_directories( "C:/Program Files (x86)/Agilent/Acqiris/lib" )
  add_library( AqDrv4 SHARED IMPORTED )
  set_target_properties( AqDrv4 PROPERTIES
    IMPORTED_IMPLIB "C:/Program Files (x86)/Agilent/Acqiris/lib/AqDrv4_x64.lib" )
endif()

target_link_libraries( ap240 PUBLIC
  adlog
  adfs
  adutils
  adportable
  adcontrols 
  AqDrv4
  ${Boost_LIBRARIES}
  ${platform_libs}
  )
