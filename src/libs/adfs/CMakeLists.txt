# adfs

project( adfs )

cmake_policy( SET CMP0022 NEW )

set( SHARED_ADFS_LIBRARY ON )

if ( SHARED_ADFS_LIBRARY )
  find_package(Threads REQUIRED)
  add_definitions( -DADFS_SHARED_LIBRARY -DADFS_LIBRARY )
endif()

if ( MSVC )
  add_definitions( -D_SCL_SECURE_NO_WARNINGS -wd4669 )
endif()

add_definitions( -DSQLITE_THREADSAFE )
add_definitions( -DLOG_ADLOG=1 )

if ( WIN32 )
  set( adfs_SOURCE apiwin32.cpp apiwin32.hpp )
  add_definitions( "-DSQLITE_API=__declspec (dllexport)" )
else()
  set( adfs_SOURCE apiposix.cpp apiposix.hpp )
  set( oslibs ${CMAKE_DL_LIBS} Threads::Threads )
endif()

add_library( ${PROJECT_NAME} SHARED
  adfs.cpp
  attributes.cpp
  filesystem.cpp
  fs.cpp
  folder.cpp
  file.cpp
  sqlite.cpp
  sqlite-amalgamation-3370000/sqlite3.c
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

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adlog
  adportable
  adportable_serializer
  ${oslibs}
  ${Boost_LIBRARIES}
  )

add_library( sqlite3-functions MODULE extension-functions.c )
set_target_properties( sqlite3-functions PROPERTIES  PREFIX "" )

add_executable( sqlite3 sqlite-amalgamation-3370000/sqlite3.c sqlite-amalgamation-3370000/shell.c )

find_package( Threads REQUIRED )
target_link_libraries( sqlite3 Threads::Threads ${CMAKE_DL_LIBS} )

runtime_install_path( _runtime_destination ${PROJECT_NAME} )
library_install_path( _library_destination ${PROJECT_NAME} )
archive_install_path( _archive_destination ${PROJECT_NAME} )

install( TARGETS ${PROJECT_NAME}
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
  LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
  ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
  )

install( TARGETS sqlite3
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
  LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
  ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
)

install( TARGETS sqlite3-functions
  EXPORT ${QTPLATZ_TARGETS}
  RUNTIME DESTINATION ${_runtime_destination} COMPONENT applications
  LIBRARY DESTINATION ${_library_destination} COMPONENT runtime_libraries
  ARCHIVE DESTINATION ${_archive_destination} COMPONENT libraries
)

install( FILES
  adfs_global.h
  adfs.hpp
  apiposix.hpp
  apiwin32.hpp
  attributes.hpp
  CMakeLists.txt
  cpio.hpp
  file.hpp
  filesystem.hpp
  folder.hpp
  fs.hpp
  sqlite3ext.h
  sqlite3.h
  sqlite.hpp
  DESTINATION include/adfs COMPONENT headers )
