#!/bin/bash

source ./prompt.sh

cwd=$(pwd)
arch=`uname`-`arch`
target=gnuplot
source_dir=~/src/gnuplot

echo "Install dependency"
sudo apt-get update
sudo apt-get install -y cvs autotools-dev automake

echo "$target install"

if [ ! -d $source_dir ]; then
    src=$(dirname $source_dir)
    if [ ! -d $(dirname $src) ]; then
	mkdir -p $(dirname $src)
    fi
    echo "Hit Enter when asked for a password."
    (cd $src;
     cvs -d:pserver:anonymous@gnuplot.cvs.sourceforge.net:/cvsroot/gnuplot login
     cvs -z3 -d:pserver:anonymous@gnuplot.cvs.sourceforge.net:/cvsroot/gnuplot co -P gnuplot
    )
fi

cd $source_dir

if [ -z $cross_target ]; then
    echo "BUILD_DIR : " `pwd`
    ./prepare
    ./configure
    make -j4
    echo sudo make install
    prompt
    sudo make install
fi
