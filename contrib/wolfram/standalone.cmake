
set ( STANDALONE TRUE )

find_path( _module_path "FindWolfram.cmake" PATHS "${CMAKE_SOURCE_DIR}/../../cmake/Modules" )

if ( _module_path )
  set( CMAKE_MODULE_PATH ${_module_path} )
else()
  message( FATAL "CMAKE_MODULE_PATH NOT FOUND" )
endif()

if ( APPLE )
  set( CMAKE_SHARED_MODULE_SUFFIX ".dylib" )
  set( CMAKE_MACOSX_RPATH 1 )
endif()
