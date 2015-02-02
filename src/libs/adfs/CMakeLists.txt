# adfs

project( adfs )

add_definitions(-DSQLITE_THREADSAFE)

if ( WIN32 )
  set( adfs_SOURCE apiwin32.cpp apiwin32.hpp )
else()
  set( adfs_SOURCE apiposix.cpp apiposix.hpp )
endif()
  
add_library( adfs 
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

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin COMPONENT applications
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )
