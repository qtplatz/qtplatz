#!/bin/bash

arch=host
ace_version=6.2.8
qwt_version=6.1.2-svn
boost_version=1_56

echo $#

if [ $# -ge 1 ]; then
  arch=$1
fi

echo "arch=" $arch

case $arch in
pi)
   export QTDIR=/opt/pi/qt5/bin
   export BOOST_ROOT=/nfs/local/pi/boost-$boost_version
   export ACE_ROOT=/nfs/local/pi/ace+tao/$ace_version
   export QWT=/nfs/local/pi/$qwt_version
   export PATH=$QTDIR:/usr/local/ace+tao/$ace_version/bin:$PATH
   xterm -bg lightcyan &
   ;;

host)
   export QTDIR=/opt/Qt5.3.2/5.3/gcc_64
   export BOOST_ROOT=/usr/local/boost-$boost_version
   export ACE_ROOT=/usr/local/ace+tao/$ace_version
   export QWT=/usr/local/qwt-$qwt_version
   export PATH=$QTDIR/bin:$ACE_ROOT/bin:$PATH
   xterm -bg ivory &
   ;;
esac


