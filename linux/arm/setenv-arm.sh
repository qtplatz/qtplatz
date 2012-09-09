export TARGET_PREFIX=arm-linux-gnueabi-
export ARCH=${TARGET_PREFIX%%-*}
export CROSS_COMPILE=${TARGET_PREFIX}
export CC=arm-linux-gnueabi-gcc
export CXX=arm-linux-gnueabi-g++
export GDB=arm-linux-gnueabi-gdb
