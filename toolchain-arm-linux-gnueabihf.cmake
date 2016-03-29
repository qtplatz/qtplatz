
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

SET(CMAKE_C_COMPILER   arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# where is the target environment
get_filename_component (_srcdir "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component (_bindir "${CMAKE_BINARY_DIR}" PATH )
set ( CMAKE_FIND_ROOT_PATH  /usr/local/arm-linux-gnueabihf ${_bindir} ${_srcdir} )

# search for programs in the build host directories
set ( CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER )

# for libraries and headers in the target directories
## multi-arch need this to both due to /usr/lib/<arch>-linux-gnueabihf
set ( CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY ) 
set ( CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY )
