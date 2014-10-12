#!/bin/bash

arch=`uname -m`
ace_version=6.2.8
qwt_version=6.1.2-svn
boost_version=1_56
xlocal=/nfs/pi-wheezy
xtools=/nfs/home/rpi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64
xarch=

if [ $# -ge 1 ]
then
    xarch=$1
    echo "seting cross build environment for target: " $xarch " on " $arch
    case $xarch in
	pi)
	    export QTDIR=/opt/qt5-rpi
	    export BOOST_ROOT=$xlocal/boost-$boost_version
	    export ACE_ROOT=$xlocal/ace+tao/$ace_version
	    export QWT=$xlocal/qwt-$qwt_version
            export RDBASE=$xlocal
	    export PATH=$QTDIR/bin:/usr/local/ace+tao/$ace_version/bin:$PATH
	    export PATH=$PATH:$xtools/bin
	    export XMLPATTERNS=/opt/Qt5.3.2/5.3/gcc_64/bin/xmlpatterns
	    xterm -bg darkmagenta -fg floralwhite &
	    ;;
    esac
else
    echo "seting self build environment for target: " $xarch " on " $arch
    case $arch in
	armv6l)
	    export QTDIR=/opt/qt5-rpi
	    export BOOST_ROOT=/usr/local/boost-$boost_version
	    export ACE_ROOT=/usr/local/ace+tao/$ace_version
	    export QWT=/usr/local/qwt-$qwt_version
	    export RDBASE=/usr/local
	    export PATH=$QTDIR/bin:/usr/local/ace+tao/$ace_version/bin:$PATH
	    xterm -bg floralwhite -fg darkmagenta &
	    ;;
	
	x86_64)
	    export QTDIR=/opt/Qt5.3.2/5.3/gcc_64
	    export BOOST_ROOT=/usr/local/boost-$boost_version
	    export ACE_ROOT=/usr/local/ace+tao/$ace_version
	    export QWT=/usr/local/qwt-$qwt_version
	    export RDBASE=/usr/local
	    export PATH=$QTDIR/bin:$ACE_ROOT/bin:$PATH
	    xterm -bg ivory &
	    ;;
	
	i686)
	    export QTDIR=/opt/Qt5.3.2/5.3/gcc
	    export BOOST_ROOT=/usr/local/boost-$boost_version
	    export ACE_ROOT=/usr/local/ace+tao/$ace_version
	    export QWT=/usr/local/qwt-$qwt_version
	    export RDBASE=/usr/local
	    export PATH=$QTDIR/bin:$ACE_ROOT/bin:$PATH
	    xterm -bg lavender &
	    ;;

    esac
fi



