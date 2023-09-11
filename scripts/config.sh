#!/bin/bash

cwd="$( cd "$( dirname "$0" )" && pwd )"
OS="$(uname -s)"

if [ -z $BOOST_VERSION ]; then
	case "${OS}" in
#		Linux) BOOST_VERSION=1_78_0;; # workaround for z440 (1.79 does not build libboost_json.so)
		*) BOOST_VERSION=1_79_0;; # require Qt5.15.2 or higher
	esac
fi

if [ -z $BOOST_ROOT ]; then
    a=(${BOOST_VERSION//_/ })
    BOOST_ROOT=/usr/local/boost-${a[0]}_${a[1]}
fi

if [ -z $CMAKE_VERSION ]; then
	CMAKE_VERSION=3.27.4
fi

if [ -z $SRC ]; then
    SRC=~/src
fi

if [ -z $DOWNLOADS ]; then
    DOWNLOADS=~/Downloads
fi

. ${cwd}/find_qmake.sh
