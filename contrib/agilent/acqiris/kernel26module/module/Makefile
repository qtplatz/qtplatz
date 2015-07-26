# Makefile for building the acqiris kernel module


obj-m := acqiris.o
acqiris-objs := acqiris-init.o acqiris-ioctl.o acqiris-dma.o


EXTRA_CFLAGS += -Wall


all install:
	@echo "Error: Cannot compile from here. Compile from parent directory."
	@false


clean:
	rm -f *~ core .*.cmd *.o *.ko modules.order acqiris.mod.c Module.symvers -r .tmp*



