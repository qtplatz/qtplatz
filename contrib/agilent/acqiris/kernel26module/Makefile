# Makefile for building the Kernel-Space Device Driver

ifndef KERNELVERSION
KERNELVERSION := $(shell uname -r)
endif

ifndef KERNELBUILD
KERNELBUILD=/lib/modules/$(KERNELVERSION)/build
endif


KDIR := $(KERNELBUILD)
MDIR := $(shell pwd)/module


EXTRA_CFLAGS += 


all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(MDIR) modules

install:
	$(MAKE) -C $(KDIR) SUBDIRS=$(MDIR) modules_install
	depmod -a
	install -m 755 init.d/acqiris /etc/init.d/acqiris
	if test -e /usr/sbin/update-rc.d ; then update-rc.d acqiris defaults 98 02 ; fi
	if test -e /sbin/chkconfig ; then chkconfig --add acqiris ; fi
	if test -e /sbin/chkconfig ; then chkconfig --level 2345 acqiris on ; fi
	/etc/init.d/acqiris start


uninstall:
	/etc/init.d/acqiris stop
	rm -f $(dir $(KDIR))/extra/acqiris.ko
	depmod -a
	rm -f /etc/init.d/acqiris
	if test -e /usr/sbin/update-rc.d ; then update-rc.d acqiris remove ; fi
	if test -e /sbin/chkconfig ; then chkconfig --del acqiris ; fi

clean:
	$(MAKE) -C $(MDIR) clean



