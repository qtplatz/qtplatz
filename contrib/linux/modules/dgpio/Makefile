# Makefile
KBUILD_CFLAGS += -std=gnu99 -O2
ARCH := $(shell arch)
ccflags-y += -std=gnu99 -O2  -Wno-declaration-after-statement
ccflags-y += -DMOD_VERSION=\"${VERSION}\"

ifeq (${VERSION},)
	VERSION := $(shell git describe --dirty --always --tags)
endif

ifeq ($(KERNELRELEASE),)

REVISION = $(shell uname -r)

# normal Makefile

KDIR := /lib/modules/`uname -r`/build

all:
	$(MAKE) -C $(KDIR) M=`pwd` CONFIG_DGPIO=m VERSION=\"${VERSION}\" modules

install:
	cp -p dgpio.ko /lib/modules/${REVISION}/extra
	/sbin/depmod -a

install-init.d:
	install -m 755 init.d-dgpio.sh /etc/init.d/dgpio
	if test -e /usr/sbin/update-rc.d ; then update-rc.d dgpio defaults 98 02 ; fi

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
