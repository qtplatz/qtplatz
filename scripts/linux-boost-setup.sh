#!/bin/bash

cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/constants.sh
source ${cwd}/prompt.sh
source ${cwd}/nproc.sh

arch=`uname`-`arch`

function bzip2_download {
    BZIP2_SOURCE=$1
    DOWNLOADS=$2
    if [ ! -d $BZIP2_SOURCE ]; then
		if [ ! -f ${DOWNLOADS}/bzip2-1.0.6.tar.gz ]; then
			if [ ! -d ${DOWNLOADS} ]; then
				mkdir ${DOWNLOADS}
			fi
			curl -L -o ${DOWNLOADS}/bzip2-1.0.6.tar.gz http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz
		fi
		if [ ! -f $(dirname $BZIP2_SOURCE) ]; then
			mkdir -p $(dirname $BZIP2_SOURCE)
			tar xvf ${DOWNLOADS}/bzip2-1.0.6.tar.gz -C $(dirname $BZIP2_SOURCE)
		fi
    fi
}

function boost_download {
    BOOST_VERSION=$1
    BUILD_ROOT=$2
    BOOST_BUILD_DIR=$3

    if [ ! -f ${DOWNLOADS}/boost-${BOOST_VERSION}.tar.bz2 ]; then
		echo "=============================="
		VERSION=$(echo $BOOST_VERSION | tr _ .)
		echo curl -L -o ${DOWNLOADS}/boost-${BOOST_VERSION}.tar.bz2 https://sourceforge.net/projects/boost/files/boost/$VERSION/boost_$BOOST_VERSION.tar.bz2/download
		curl -L -o ${DOWNLOADS}/boost-${BOOST_VERSION}.tar.bz2 https://sourceforge.net/projects/boost/files/boost/$VERSION/boost_$BOOST_VERSION.tar.bz2/download
	fi
	if [ ! -f ${BOOST_BUILD_DIR} ]; then
		tar xvf ${DOWNLOADS}/boost-${BOOST_VERSION}.tar.bz2 -C $(dirname ${BOOST_BUILD_DIR})
	fi
#	if [ -f ${BOOST_BUILD_DIR} ]; then
#		rm -rf ${BOOST_BUILD_DIR}
#	fi
}

function boost_build {
    echo "=============================="
    echo "   BOOST install for $arch    "
    echo "=============================="

    BOOST_BUILD_DIR=$1
    BZIP2_SOURCE=$2

    ( cd $BOOST_BUILD_DIR;
      echo $(pwd)

      case "${arch}" in
		  Linux*)
			  echo "********************************************"
			  echo "It seems that libbz2.a should be removed befor run following"
			  echo "********************************************"
			  PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
			  PYTHON_ROOT=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"data\"])")
			  PYTHON=$(python3 -c "import sys; print(sys.executable)")
			  echo ./bootstrap.sh --prefix=$BOOST_PREFIX --with-python=${PYTHON}
			  echo ./b2 -j $nproc address-model=64 toolset=gcc threading=multi cflags=-fPIC cxxflags="-fPIC -std=c++17" include=${PYTHON_INCLUDE}
			  prompt
			  ./bootstrap.sh --prefix=$BOOST_PREFIX --with-python=${PYTHON} &&
			  	  ./b2 -j $nproc address-model=64 cflags=-fPIC cxxflags="-fPIC -std=c++17" include=${PYTHON_INCLUDE} # -sBZIP2_SOURCE=${BZIP2_SOURCE}
			  ;;
		  Darwin*)
			  echo "***********************************************************************************************************"
			  echo "if you got failed by zlib, try following command."
			  echo "sudo installer -pkg /Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_10.14.pkg -target /"
			  echo "***********************************************************************************************************"
			  PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
			  PYTHON_ROOT=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"data\"])")
			  PYTHON=$(python3 -c "import sys; print(sys.executable)")
			  OSX_VERSION_MIN=-mmacosx-version-min=10.12
			  #CXX_FLAGS="-std=c++17 $OSX_VERSION_MIN"
			  #LINKFLAGS="-stdlib=libc++ $OSX_VERSION_MIN"
			  CXX_FLAGS="-std=c++17"
			  LINKFLAGS="-stdlib=libc++"
			  echo ./bootstrap.sh --prefix=$BOOST_PREFIX --with-toolset=clang --with-python=${PYTHON} --with-python-root=${PYTHON_ROOT} --with-python-version=3.7
			  prompt
			  ./bootstrap.sh --prefix=$BOOST_PREFIX --with-toolset=clang --with-python=${PYTHON} --with-python-root=${PYTHON_ROOT} --with-python-version=3.7
			  echo ./b2 -j $nproc address-model=64 toolset=clang cxxflags="$CXX_FLAGS" linkflags="$LINKFLAGS" include=${PYTHON_INCLUDE}
			  prompt
			  ./b2 -j $nproc address-model=64 toolset=clang cxxflags="$CXX_FLAGS" linkflags="$LINKFLAGS" include=${PYTHON_INCLUDE}
			  ;;
		  *)
			  echo "Unknown arch: " $arch
      esac

      echo "*****************************************************"
      echo "boost has been built on `pwd`";
      echo "run following command to install"
      echo "cd `pwd`"
	  echo "sudo ./b2 install"
      echo "*****************************************************"
      prompt

      case "${arch}" in
	  Linux*)
		  ./b2 -j $nproc address-model=64 cflags=-fPIC cxxflags='"-fPIC -std=c++14"' -s BZIP2_SOURCE=${BZIP2_SOURCE} install
	      ;;
	  Darwin*)
	      sudo ./b2 install
	      ;;
	  *)
	      echo "Unknown arch: " $arch
      esac
    )
}

function boost_cross_build {
    echo "=============================="
    echo "   BOOST cross install for $cross_target "
    echo "=============================="

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
      echo ./b2 toolset=gcc-arm -s BZIP2_SOURCE=$BZIP2_SOURCE -j $nproc install
      prompt

      ./bootstrap.sh --prefix=$BOOST_PREFIX &&
	  ./b2 toolset=gcc-arm -s BZIP2_SOURCE=$BZIP2_SOURCE -j $nproc install
    )
}

if [ -z $BOOST_VERSION ]; then
    BOOST_VERSION=1_67_0
fi

if [ -z $PREFIX ]; then
    PREFIX=/usr/local
fi

if [ -z $cross_target ]; then
    BUILD_ROOT=$SRC/build-$arch
    CROSS_ROOT=
else
    BUILD_ROOT=$SRC/build-$cross_target;
    CROSS_ROOT=/usr/local/arm-linux-gnueabihf
fi

BZIP2_SOURCE=$SRC/bzip2-1.0.6
BOOST_BUILD_DIR=$BUILD_ROOT/boost_${BOOST_VERSION}
BOOST_PREFIX=${CROSS_ROOT}$PREFIX/boost-${BOOST_VERSION/%_0//}

echo "INSTALLING 'boost' $cross_target to"
echo "	BOOST DOWNLOAD    : ${DOWNLOADS}"
echo "	BOOST_BUILD_DIR   : ${BOOST_BUILD_DIR}"
echo "	BOOST_PREFIX      : ${BOOST_PREFIX}"
echo "	CROSS_ROOT        : ${CROSS_ROOT}"

if [[ "$OSTYPE" == "darwin"* ]]; then
	echo "	BZIP2_SOURCE      : Using OSX native"
else
	if [ ! -d ${BZIP2_SOURCE} ]; then
		echo "	BZIP2_SOURCE      : download to ${BZIP2_SOURCE}"
	else
		echo "	BZIP2_SOURCE      : ${BZIP2_SOURCE}"
	fi
fi

if [ ! -d $BOOST_BUILD_DIR ]; then
    echo "	BOOST_BUILD_DIR   : download to ${BOOST_BUILD_DIR}"
else
    echo "	BOOST_BUILD_DIR   : ${BOOST_BUILD_DIR}"
fi

if [ -d ${BOOST_PREFIX} ]; then
    echo "boost-$BOOST_VERSION already installed in ${BOOST_PREFIX}"
fi

__nproc nproc
echo "NPROC=" $nproc
echo "CWD=" $cwd

prompt

if [[ "$OSTYPE" != "darwin"* ]] && [ ! -d ${BZIP2_SOURCE} ]; then
    bzip2_download $BZIP2_SOURCE $DOWNLOADS
    if [ ! $? -eq 0 ]; then exit 1; fi
fi

if [ ! -d ${BUILD_ROOT} ]; then
    mkdir -p ${BUILD_ROOT}
fi

boost_download ${BOOST_VERSION} ${BUILD_ROOT} ${BOOST_BUILD_DIR}

if [ -z $cross_target ]; then

	#	if [ -f ~/user-config.jam ]; then
	#		mv ~/user-config.jam ~/user-config.jam.orig
	#	fi
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
	    exit 1
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
