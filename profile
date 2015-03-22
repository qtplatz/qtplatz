#!/bin/bash

case "$1" in
    pi)
	export cross_target=raspi
	export prefix=/opt/raspi
	export rootfs=/opt/raspi/arm-linux-gnueabihf-rootfs
	export destdir=/opt/raspi/arm-linux-gnueabihf
	tooldir=/opt/raspi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin
	export PATH=/opt/qt5pi/bin:/usr/local/ace+tao/6.3.0/bin:$tooldir:$PATH
	export PS1="\u@\h(pi) $ "
	alias xterm='xterm -bg Thistle'
	echo "***********************************"
	echo "Environment set up for Raspberry-Pi"
	echo "***********************************"
	;;
    helio|soc)
	export cross_target=helio
	prefix=/usr/local
	export rootfs=/opt/local/arm-linux-gnueabihf-rootfs
	export PATH=$prefix/arm-linux-gnueabihf/usr/local/qt5/bin:/usr/local/ace+tao/6.3.0/bin:$PATH
	export PS1="\u@\h(helio) $ "	
	alias xterm='xterm -bg Cornsilk1' 
	echo "*******************************************"
	echo "Environment set up for Helio (Cycron V SoC)"
	echo "*******************************************"
	;;
    *)
	echo "Usage: $0 pi|helio" >&2
	exit 3
	;;
esac



