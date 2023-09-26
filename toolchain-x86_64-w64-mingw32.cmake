
set ( CMAKE_SYSTEM_NAME Linux )
set ( CMAKE_SYSTEM_VERSION 1 )

set ( CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc )
set ( CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++ )

set ( CMAKE_STAGING_PREFIX /usr/local/x86_64-w64-mingw32/usr/local )

# where is the target environment
get_filename_component (_srcdir "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component (_bindir "${CMAKE_BINARY_DIR}" PATH )
set ( CMAKE_FIND_ROOT_PATH  /usr/local/x86_64-w64-mingw32 /usr/x86_64-w64-mingw32 ${_bindir} ${_srcdir} )

# search for programs in the build host directories
set ( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )

set ( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY )
set ( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
