#!/bin/bash

source ./constants.sh
source ./prompt.sh

QWT_BUILD_DIR=${SRC}/qwt-6.1

if ! find_QMAKE QMAKE; then
    echo "qmake command not found."
    exit
fi

echo "==========================="
echo "Install qwt using $QMAKE"
prompt
   
if ! type svn >/dev/null; then
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

$QMAKE -r qwt.pro
make -j4
echo "==========================="
echo "sudo make install on `pwd`"
sudo make install

