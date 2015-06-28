cmake_minimum_required( VERSION 3.1 )
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0020 NEW)

project( bdfconv )

if ( NOT WIN32 )
  message( "ONLY WIN32 x86" )
endif()

set( QTPLATZ_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../.. )
include ( "../../../qtplatz.cmake" )

if ( MSVC )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS -wd4996 )
endif()

find_package(Boost 1.57 REQUIRED COMPONENTS date_time filesystem program_options serialization system )

include_directories( ${Boost_INCLUDE_DIRS} ${QTPLATZ_SOURCE_DIR}/src/libs )

if ( WIN32 )
  set( platform_SOURCE ../../../src/libs/adfs/apiwin32.cpp )
else()
  set( platform_SOURCE ../../../src/libs/adfs/apiposix.cpp )
endif()

add_executable( bdfconv
  main.cpp
  ../../../src/libs/adfs/adfs.cpp
  ../../../src/libs/adfs/sqlite.cpp
  ../../../src/libs/adfs/sqlite.hpp
  ../../../src/libs/adfs/sqlite3.c
  ../../../src/libs/adportable/ConvertUTF.c 
  ../../../src/libs/adportable/debug.cpp
  ../../../src/libs/adportable/debug_core.cpp
  ../../../src/libs/adportable/profile.cpp  
  ../../../src/libs/adportable/string.cpp
  ../../../src/libs/adportable/utf.cpp
  ../../../src/libs/adportable/uuid.cpp
  ${platform_SOURCE}
  )

target_link_libraries( bdfconv ${Boost_LIBRARIES} )
