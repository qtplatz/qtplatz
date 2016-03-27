SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

#SET(CMAKE_C_COMPILER   /opt/raspi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-gcc)
#SET(CMAKE_CXX_COMPILER /opt/raspi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin/arm-linux-gnueabihf-g++)
SET(CMAKE_C_COMPILER   arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
SET(CMAKE_C_FLAGS "-march=armv6" CACHE STRING "Raspbery Pi")
SET(CMAKE_CXX_FLAGS "-march=armv6" CACHE STRING "Raspbery Pi")

# where is the target environment
get_filename_component (_srcdir "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component (_bindir "${CMAKE_BINARY_DIR}" PATH )
SET( CMAKE_FIND_ROOT_PATH /opt/raspi/arm-linux-gnueabihf /opt/raspi/arm-linux-gnueabihf-rootfs ${_bindir} ${_srcdir} )
set( CMAKE_SYSROOT /opt/raspi/arm-linux-gnueabihf-rootfs )

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

