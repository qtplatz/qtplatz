#!/bin/bash

SRC=~/src
QWT_BUILD_DIR=${SRC}/qwt-6.1

subversion = $(dpkg -l subversion | grep subversion | wc -l)

if ! which svn >/dev/null; then
    sudo apt-get -y install subversion
fi

if [ ! -d $QWT_BUILD_DIR ]; then
    svn checkout svn://svn.code.sf.net/p/qwt/code/branches/qwt-6.1 $QWT_BUILD_DIR
fi

cd $QWT_BUILD_DIR
svn update

cp -p qwtconfig.pri qwtconfig.pri.orig

sed -i '/QwtDll/s/^/#/' qwtconfig.pri
sed -i '/QwtMathML/s/^/#/' qwtconfig.pri
sed -i '/QwtDesigner/s/^/#/' qwtconfig.pri
sed -i '/QwtExamples/s/^/#/' qwtconfig.pri
sed -i '/QwtPlayground/s/^/#/' qwtconfig.pri

qmake -r qwt.pro
make -j4
sudo make install

