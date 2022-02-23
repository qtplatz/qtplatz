#!/bin/bash

cwd="$(cd "$(dirname "$0")" && pwd)"
arch=`uname`-`arch`
source ${cwd}/config.sh
source ./prompt.sh

function qwt_download {
    QWT_VERSION=$1
    QWT_BUILD_DIR=$2

    if [ ! -f ${DOWNLOADS}/qwt-${QWT_VERSION}.tar.bz2 ]; then
		echo "=============================="
		VERSION=$(echo $QWT_VERSION | tr _ .)
		echo curl -L -o ${DOWNLOADS}/qwt-${QWT_VERSION}.tar.bz2 https://sourceforge.net/projects/qwt/files/qwt/6.2.0/qwt-${QWT_VERSION}.tar.bz2/download
		curl -L -o ${DOWNLOADS}/qwt-${QWT_VERSION}.tar.bz2 https://sourceforge.net/projects/qwt/files/qwt/6.2.0/qwt-${QWT_VERSION}.tar.bz2/download
	fi

	if [ ! -d ${QWT_BUILD_DIR} ]; then
		prompt
		tar xvf ${DOWNLOADS}/qwt-${QWT_VERSION}.tar.bz2 -C $(dirname ${QWT_BUILD_DIR})
	fi
}

QWT_VERSION=6.2.0
QWT_BUILD_DIR=${SRC}/qwt-6.2.0

if ! find_qmake QMAKE; then
    echo "qmake command not found."
    exit
fi

echo "==========================="
echo "Install qwt using $QMAKE"
prompt

qwt_download ${QWT_VERSION} ${QWT_BUILD_DIR}

case $arch in
	Darwin-i386)
		qmake_args=('QMAKE_APPLE_DEVICE_ARCHS=x86_64')
		;;
	Darwin-arm64)
		qmake_args=('QMAKE_APPLE_DEVICE_ARCHS=arm64')
		;;
esac

cd ${QWT_BUILD_DIR}

cp -p qwtconfig.pri qwtconfig.pri.orig

cat qwtconfig.pri.orig | \
	sed '/QwtDll/s/^/#/' | \
	sed '/QwtMathML/s/^/#/' | \
	sed '/QwtDesigner/s/^/#/' | \
	sed '/QwtExamples/s/^/#/' | \
	sed '/QwtFramework/s/^/#/' | \
	sed '/QwtPlayground/s/^/#/' > qwtconfig.pri

echo $QMAKE -r qwt.pro "${qmake_args[@]}"

$QMAKE -r qwt.pro "${qmake_args[@]}"
make -j4
echo "==========================="
echo "sudo make install on `pwd`"
sudo make install
