KBUILD_CFLAGS += -std=gnu99
#KERNEL_SRC = /home/toshi/src/armhf/linux-4.0.5
KERNEL_SRC = /home/toshi/src/linux-4.4.6

obj-m += dgirq.o

all:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C ${KERNEL_SRC} M=${PWD} modules

clean:
	make -C ${KERNEL_SRC} m=${PWD} clean
