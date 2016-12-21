cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0022 NEW )
cmake_policy( SET CMP0028 NEW )

project( ap240 )

if ( NOT Boost_FOUND )
  #find_package(Boost 1.57 REQUIRED COMPONENTS date_time regex system filesystem wserialization serialization program_options bzip2 iostreams )
endif()

add_definitions(-DAP240_LIBRARY -D_SCL_SECURE_NO_WARNINGS)

set( platform_libs "" )

if ( ${CMAKE_SYSTEM} MATCHES "Linux" )
  set( platform_libs dl )
  add_definitions( -D_LINUX -D_ACQIRIS )
endif()

set( ap240_SOURCES
  ap240_global.hpp
  digitizer.cpp
  digitizer.hpp
  safearray.hpp
  )

add_library( ap240 SHARED  ${ap240_SOURCES}  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR} 
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${AqDrv4_INCLUDE_DIR}
  )

if ( WIN32 )
  add_definitions( -wd4996 )
  include_directories( "${AqDrv4_INCLUDE_DIR}" )
  #link_directories( "C:/Program Files (x86)/Agilent/Acqiris/lib" )
  #add_library( AqDrv4 SHARED IMPORTED ) --> FindAqDrv4.cmake
  #set_target_properties( AqDrv4 PROPERTIES
  #  IMPORTED_IMPLIB "C:/Program Files (x86)/Agilent/Acqiris/lib/AqDrv4_x64.lib" )
endif()

target_link_libraries( ap240 PUBLIC
  adlog
  adfs
  adutils
  adportable
  adcontrols
  acqrscontrols
  AqDrv4
  ${Boost_LIBRARIES}
  ${platform_libs}
  )

add_executable( test_digitizer main.cpp )

target_link_libraries( test_digitizer LINK_PUBLIC
  ${PROJECT_NAME}
  ${Boost_LIBRARIES}
  ${platform_libs}
)  

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT modules
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
  
