KBUILD_CFLAGS += -std=gnu99
REVISION = `uname -r`
KERNEL_SRC = ~/src/linux-${REVISION}

obj-m += dgmod.o

all:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C ${KERNEL_SRC} M=${PWD} modules

clean:
	-rm -f *.o *.mod.* *~
#	make -C ${KERNEL_SRC} m=${PWD} clean

install:
	sudo cp -p dgmod.ko /lib/modules/${REVISION}/extra

start:
	sudo insmod dgmod.ko

stop:
	sudo rmmod dgmod

