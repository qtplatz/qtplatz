#!/bin/bash

cwd="$( cd "$( dirname "$0" )" && pwd )"

if [ -z $BOOST_VERSION ]; then
    BOOST_VERSION=1_62_0
fi
if [ -z $BOOST_ROOT ]; then
    a=(${BOOST_VERSION//_/ })
    BOOST_ROOT=/usr/local/boost-${a[0]}_${a[1]}
fi

if [ -z $SRC ]; then
    SRC=~/src
fi

if [ -z $DOWNLOADS ]; then
    DOWNLOADS=~/Downloads
fi

. ${cwd}/find_qmake.sh
