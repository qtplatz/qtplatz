SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

SET(CMAKE_C_COMPILER   arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
SET(CMAKE_AR           arm-linux-gnueabihf-ar)
SET(CMAKE_LINKER       arm-linux-gnueabihf-ld)
SET(CMAKE_NM           arm-linux-gnueabihf-nm)
SET(CMAKE_OBJCOPY      arm-linux-gnueabihf-objcopy)
SET(CMAKE_OBJDUMP      arm-linux-gnueabihf-objdump)
SET(CMAKE_STRIP        arm-linux-gnueabihf-strip)
SET(CMAKE_RANLIB       arm-linux-gnueabihf-tanlib)

# where is the target environment 
SET(CMAKE_FIND_ROOT_PATH  /mnt/rootfs /nfs/local/pi)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

######
set(Boost_USE_STATIC_LIBS ON)
set(Boost_INCLUDE_DIR "/nfs/local/pi/boost-1_56/include")
set(Boost_LIBRARY_DIR "/nfs/local/pi/boost-1_56/lib")
#
