#!/bin/bash
arch=`uname`-`arch`

function prompt {
    while true; do
	read -p "Proceed (y/n)? " yn
	case $yn in
            [Yy]* ) break;;
            [Nn]* ) exit;;
            * ) echo "Please answer yes or no.";;
	esac
    done
}

function bzip2_download {
    BZIP2_SOURCE=$1
    DOWNLOADS=$2
    if [ ! -d $BZIP2_SOURCE ]; then
	if [ ! -f ${DOWNLOADS}/bzip2-1.0.6.tar.gz ]; then
	    (cd ${DOWNLOADS}; 
	     wget http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz )
	fi
	if [ ! -f $(dirname $BZIP2_SOURCE) ]; then
	    mkdir -p $(dirname $BZIP2_SOURCE)
	fi
	tar xvf ${DOWNLOADS}/bzip2-1.0.6.tar.gz -C $(dirname $BZIP2_SOURCE)
    fi
}

function boost_download {
    BOOST_VERSION=$1
    BUILD_ROOT=$2
    BOOST_BUILD_DIR=$3

    if [ ! -f ${DOWNLOADS}/boost-${BOOST_VERSION}.tar.bz2 ]; then
	echo "=============================="
	VERSION=$(echo $BOOST_VERSION | tr _ .)    
	echo wget https://sourceforge.net/projects/boost/files/boost/$VERSION/boost_$BOOST_VERSION.tar.bz2/download
	if [ -f download ]; then
	    echo "Clean previoud download file"
	    rm -f download
	fi
	wget https://sourceforge.net/projects/boost/files/boost/$VERSION/boost_$BOOST_VERSION.tar.bz2/download
	mv download ${DOWNLOADS}/boost-${BOOST_VERSION}.tar.bz2
    fi
    if [ -f ${BOOST_BUILD_DIR} ]; then
	rm -rf ${BOOST_BUILD_DIR}
    fi
    tar xvf ${DOWNLOADS}/boost-${BOOST_VERSION}.tar.bz2 -C $(dirname $BOOST_BUILD_DIR)	
}

function boost_build {
    echo "=============================="
    echo "   BOOST install for $arch    "
    echo "=============================="
    
    BOOST_BUILD_DIR=$1
    BZIP2_SOURCE=$2
    ( cd $BOOST_BUILD_DIR;
      echo $(pwd)
      echo ./bootstrap.sh --prefix=$BOOST_PREFIX
      echo ./b2 -j4 address-model=64 cflags=-fPIC cxxflags="-fPIC -std=c++11" -s BZIP2_SOURCE=${BZIP2_SOURCE}
      prompt
      
      ./bootstrap.sh --prefix=$BOOST_PREFIX &&
	  ./b2 -j4 address-model=64 cflags=-fPIC cxxflags="-fPIC -std=c++11" -s BZIP2_SOURCE=${BZIP2_SOURCE}
	echo "*****************************************************"
	echo "boost has been built on `pwd`";
	echo "run following command to install"
	echo "cd `pwd`"
	echo "sudo ./b2 -j4 address-model=64 cflags=-fPIC cxxflags='"-fPIC -std=c++11"' -s BZIP2_SOURCE=${BZIP2_SOURCE} install"
	echo "*****************************************************"
	prompt
	sudo ./b2 -j4 address-model=64 cflags=-fPIC cxxflags='"-fPIC -std=c++11"' -s BZIP2_SOURCE=${BZIP2_SOURCE} install
    )
}

function boost_cross_build {
    echo "=============================="
    echo "   BOOST install for $cross_target "
    echo "=============================="
    prompt
    
    BOOST_BUILD_DIR=$1
    BZIP2_SOURCE=$2    
    if [ ! -d $(dirname $BOOST_PREFIX) ]; then
	if ! mkdir -p $(dirname $BOOST_PREFIX) ; then
	    echo "mkdir -p $(dirname BOOST_PREFIX) -- command faild. Check for your access permission"
	    exit
	fi
    fi

    if [ ! -w $(dirname $BOOST_PREFIX) ]; then
	echo "Make $(dirname $BOOST_PREFIX) writable."
    fi

    ( cd $BOOST_BUILD_DIR;
      echo $(pwd)
      echo ./bootstrap.sh --prefix=$BOOST_PREFIX
      echo ./b2 toolset=gcc-arm -s BZIP2_SOURCE=$BZIP2_SOURCE -j4 install
      prompt
      
      ./bootstrap.sh --prefix=$BOOST_PREFIX &&
	  ./b2 toolset=gcc-arm -s BZIP2_SOURCE=$BZIP2_SOURCE -j4 install
    )
}

if [ -z $BOOST_VERSION ]; then
    BOOST_VERSION=1_62_0
fi

if [ -z $PREFIX ]; then
    PREFIX=/usr/local
fi

if [ -z $DOWNLOADS ]; then
    DOWNLOADS=~/Downloads
fi

if [ -z $cross_target ]; then
    BUILD_ROOT=~/src/build-$arch
    CROSS_ROOT=
else
    BUILD_ROOT=~/src/build-$cross_target;
    CROSS_ROOT=/usr/local/arm-linux-gnueabihf
fi

BZIP2_SOURCE=~/src/bzip2-1.0.6
BOOST_BUILD_DIR=$BUILD_ROOT/boost_${BOOST_VERSION}
BOOST_PREFIX=${CROSS_ROOT}$PREFIX/boost-${BOOST_VERSION/%_0//}

echo "INSTALLING 'boost' $cross_target to"
echo "	BOOST DOWNLOAD    : ${DOWNLOADS}"
echo "	BOOST_BUILD_DIR   : ${BOOST_BUILD_DIR}"
echo "	BOOST_PREFIX      : ${BOOST_PREFIX}"
echo "	CROSS_ROOT        : ${CROSS_ROOT}"

if [ ! -d ${BZIP2_SOURCE} ]; then
    echo "	BZIP2_SOURCE      : download to ${BZIP2_SOURCE}"
else
    echo "	BZIP2_SOURCE      : ${BZIP2_SOURCE}"    
fi

if [ ! -d $BOOST_BUILD_DIR ]; then
    echo "	BOOST_BUILD_DIR   : download to ${BOOST_BUILD_DIR}"
else
    echo "	BOOST_BUILD_DIR   : ${BOOST_BUILD_DIR}"
fi

if [ -d ${BOOST_PREFIX} ]; then
    echo "boost-$BOOST_VERSION already installed in ${BOOST_PREFIX}"
fi

prompt

if [ ! -d ${BZIP2_SOURCE} ]; then
    bzip2_download $BZIP2_SOURCE $DOWNLOADS
    if [ ! $? -eq 0 ]; then exit 1; fi
fi

if [ ! -d ${BUILD_ROOT} ]; then
    mkdir -p ${BUILD_ROOT}
fi

boost_download ${BOOST_VERSION} ${BUILD_ROOT} ${BOOST_BUILD_DIR}

if [ -z $cross_target ]; then

    boost_build $BOOST_BUILD_DIR ${BZIP2_SOURCE}

else
    if [ ! -w $CROSS_ROOT ]; then
	echo "You have no write access to $CROSS_ROOT"
	echo "Do you want to continue with sudo?"
	prompt
	sudo mkdir -p $CROSS_ROOT
	sudo chgrp staff $CROSS_ROOT
	sudo chmod g+sw $CROSS_ROOT

	staff=$(id -Gn | grep -c staff)
	if [ $staff = 0 ]; then
	    echo "You need to join 'staff' group as run follwoing command".
	    sudo usermod -a -G staff $USER
	fi
	if [ ! -w $CROSS_ROOT ]; then
	    echo "You may need to logout/login cycle"
	    exit (1)
	fi
    fi

    if [ ! -f ~/user-config.jam ]; then
	echo "# Creating ~/user-config.jam..."
	cat <<EOF>~/user-config.jam
using gcc : arm : arm-linux-gnueabihf-g++ : <cxxflags>"-std=c++14 -fPIC" ;
using python : 2.7 ;
EOF
    fi

    boost_cross_build ${BOOST_BUILD_DIR} ${BZIP2_SOURCE}
fi

echo "=============================="
echo "   BOOST $BOOST_VERSION Installed "
echo "=============================="
