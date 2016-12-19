#
cmake_minimum_required(VERSION 2.8.11)

set( VERSION_TWEAK 0 )

execute_process( COMMAND git describe
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE git_describe
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

# git_discribe: ex v3.1.4-201-gdbc1f69
# _list may became 3; .1; .4; -201; 1; 69

string( REGEX MATCHALL "[0-9]+|[\\.\\-][0-9]+" _list ${git_describe} )

list( LENGTH _list _count )

if ( _count GREATER 1 ) # count >= 2
  string( REGEX REPLACE "v([0-9]+)\\.[0-9]+.*$" "\\1" VERSION_MAJOR ${git_describe} )
  string( REGEX REPLACE "v[0-9]+\\.([0-9]+).*$" "\\1" VERSION_MINOR ${git_describe} )
endif()

if ( _count GREATER 2 ) # count >= 3
  string( REGEX REPLACE "v[0-9]+\\.[0-9]+[\\.-]([0-9]+).*$" "\\1" VERSION_PATCH ${git_describe} )  
endif()

if ( _count GREATER 3 ) # count >= 4
  string( REGEX REPLACE "v[0-9]+\\.[0-9]+[\\.-][0-9]+-([0-9]+)-.*$" "\\1" VERSION_TWEAK ${git_describe} )
  string( REGEX REPLACE "v[0-9]+\\.[0-9]+[\\.-][0-9]+-[0-9]+-(.*)$" "\\1" VERSION_HASH ${git_describe} )    
endif()

# message( STATUS "## version: " "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}.${VERSION_TWEAK}(${VERSION_HASH})" )
