# adfs

project( adfs )

cmake_policy( SET CMP0022 NEW )

set( SHARED_ADFS_LIBRARY ON )

if ( SHARED_ADFS_LIBRARY )
  find_package(Boost 1.57 REQUIRED COMPONENTS date_time system filesystem wserialization serialization)
  add_definitions( -DADFS_SHARED_LIBRARY -DADFS_LIBRARY )
endif()

if ( MSVC )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS -wd4669 )
endif()

add_definitions(-DSQLITE_THREADSAFE)

if ( WIN32 )
  set( adfs_SOURCE apiwin32.cpp apiwin32.hpp )
else()
  set( adfs_SOURCE apiposix.cpp apiposix.hpp )
endif()
  
add_library( ${PROJECT_NAME} SHARED
  adfs.cpp
  attributes.cpp
  filesystem.cpp
  fs.cpp
  folder.cpp
  file.cpp
  sqlite.cpp
  sqlite3.c
  adfs.hpp
  attributes.hpp
  cpio.hpp
  filesystem.hpp
  fs.hpp
  folder.hpp
  file.hpp
  sqlite.hpp
  sqlite3.h
  ${adfs_SOURCE}
  )

if ( SHARED_ADFS_LIBRARY )
  target_link_libraries( adfs LINK_PUBLIC ${Boost_LIBRARIES} adportable )
endif()

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  LIBRARY DESTINATION lib/qtplatz COMPONENT libraries 
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
