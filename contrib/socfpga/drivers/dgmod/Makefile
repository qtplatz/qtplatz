KBUILD_CFLAGS += -std=gnu99
REVISION = $(shell uname -r)
KERNEL_SRC = ~/src/linux-${REVISION}

obj-m += dgmod.o

all:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C ${KERNEL_SRC} M=${PWD} modules

clean:
	-rm -f *.o *.mod.* *~
#	make -C ${KERNEL_SRC} m=${PWD} clean

install:
	cp -p dgmod.ko /lib/modules/${REVISION}/extra
	/sbin/depmod -a
#	$(MAKE) -C $(KDIR) SUBDIRS=$(MDIR) modules_install

install-init.d:
	install -m 755 init.d-dgmod.sh /etc/init.d/dgmod
	if test -e /usr/sbin/update-rc.d ; then update-rc.d dgmod defaults 98 02 ; fi

start:
	sudo insmod dgmod.ko

stop:
	sudo rmmod dgmod

