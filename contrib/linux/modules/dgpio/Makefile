# Makefile
KBUILD_CFLAGS += -std=gnu99 -O2
ARCH := $(shell arch)
ccflags-y += -std=gnu99 -O2
ccflags-y += -DMOD_VERSION=\"${VERSION}\"

ifeq (${VERSION},)
	VERSION := $(shell git describe --dirty --always --tags)
endif

ifeq ($(KERNELRELEASE),)

# normal Makefile

KDIR := /lib/modules/`uname -r`/build

all:
	$(MAKE) -C $(KDIR) M=`pwd` CONFIG_DGPIO=m VERSION=\"${VERSION}\" modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=`pwd` modules_install

clean:
	rm -f *~ core .*.cmd *.o *.ko modules.order *.mod.c Module.symvers -r .tmp*

start:
	sudo insmod dgpio.ko

stop:
	sudo rmmod dgpio

else

# kbuild part of makefile

obj-$(CONFIG_DGPIO) := dgpio.o

endif
