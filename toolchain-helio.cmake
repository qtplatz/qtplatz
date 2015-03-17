SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

SET(CMAKE_C_COMPILER   arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# where is the target environment
get_filename_component (_dir "${CMAKE_CURRENT_LIST_FILE}" PATH)
SET(CMAKE_FIND_ROOT_PATH  /opt/local/arm-linux-gnueabihf ${_dir} ${_dir}/.. )
#SET(CMAKE_FIND_ROOT_PATH  /opt/raspi/arm-linux-gnueabihf /opt/raspi/arm-linux-gnueabuhf-rootfs ${_dir} ${_dir}/.. )
foreach ( p ${CMAKE_FIND_ROOT_PATH} )
  message( "######## ROOT_PATH: " ${p} )
endforeach()

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

