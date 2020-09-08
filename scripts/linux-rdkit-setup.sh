#!/bin/bash

cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/constants.sh
source ${cwd}/prompt.sh
source ${cwd}/nproc.sh

PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
PYTHON_ROOT=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"data\"])")
PYTHON=$(python3 -c "import sys; print(sys.executable)")

arch=`uname`-`arch`

__nproc nproc

if [ -z $cross_target ]; then
    BUILD_DIR=$SRC/build-$arch/rdkit.release
    export RDBASE=$SRC/rdkit
else
    BUILD_DIR=$SRC/build-$cross_target/rdkit
    CROSS_ROOT=/usr/local/arm-linux-gnueabihf
    export RDBASE=$CROSS_ROOT/usr/local/rdkit
    TOOLCHAIN=$(dirname $cwd)/toolchain-arm-linux-gnueabihf.cmake
    if [ -z $BOOST_ROOT ]; then
		if [ -d $CROSS_ROOT/usr/local/boost-1_67 ]; then
			BOOST_ROOT=/usr/local/boost-1_67
		elif [ -d $CROSS_ROOT/usr/local/boost-1_62 ]; then
			BOOST_ROOT=/usr/local/boost-1_62
		fi
    fi
fi

if [ `uname` == "Darwin" ]; then
    cmake_args=( "-DBOOST_ROOT=$BOOST_ROOT"
			 "-DRDK_BUILD_INCHI_SUPPORT=ON"
			 "-DRDK_BUILD_PYTHON_WRAPPERS=ON"
			 "-DPYTHON_EXECUTABLE=${PYTHON}"
			 "-DPYTHON_INCLUDE_DIR=${PYTHON_INCLUDE}"
			 "-DRDK_INSTALL_INTREE=OFF"
			 "-DRDK_INSTALL_STATIC_LIBS=OFF"
			 "-DBoost_NO_BOOST_CMAKE=ON"
			 "-DRDK_BUILD_FREETYPE_SUPPORT=OFF"
			   )
	if (( $nproc < 8 )); then
		cmake_args+=(
			"-DRDK_BUILD_CPP_TESTS=OFF"
			"-DRDK_TEST_MULTITHREADED=OFF"
			"-DRDK_TEST_MMFF_COMPLIANCE=OFF"
			"-DRDK_BUILD_TEST_GZIP=OFF"
		)
	fi
    cmake_args+=("-DCMAKE_MACOSX_RPATH=TRUE")
else
    cmake_args=( "-DBOOST_ROOT=$BOOST_ROOT"
				 "-DRDK_BUILD_INCHI_SUPPORT=ON"
				 "-DRDK_BUILD_PYTHON_WRAPPERS=ON"
				 "-DPYTHON_EXECUTABLE=${PYTHON}"
				 "-DPYTHON_INCLUDE_DIR=${PYTHON_INCLUDE}"
				 "-DRDK_INSTALL_INTREE=OFF"
#				 "-DRDK_INSTALL_STATIC_LIBS=OFF"
#				 "-DRDK_INSTALL_DYNAMIC_LIBS=ON"
				 "-DBoost_NO_BOOST_CMAKE=ON"
				 "-DRDK_BUILD_FREETYPE_SUPPORT=OFF"
			   )
fi

if [ ! -z $cross_target ]; then
    cmake_args+=( "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN" "-DRDK_OPTIMIZE_NATIVE=OFF" )
	sudo apt install libeigen3-dev
fi

echo "RDKit install on $RDBASE"

if [ ! -d $RDBASE ]; then
    if [ ! -d $(dirname $RDBASE) ]; then
		mkdir -p $(dirname $RDBASE)
    fi
    git clone https://github.com/rdkit/rdkit $RDBASE
fi

mkdir -p $BUILD_DIR;
cd $BUILD_DIR;

echo "RDBASE    : " $RDBASE
echo "BUILD_DIR : " `pwd`
echo cmake "${cmake_args[@]}" $RDBASE
prompt
cmake "${cmake_args[@]}" $RDBASE
echo "make -j${nproc}"
prompt
make -k -j${nproc}

if [ $? -eq 0 ]; then
	#make test
	echo sudo make install
	prompt
	sudo make install
fi

#echo "Edit /usr/local/lib/cmake/rdkit/rdkit-config.cmake as:"
#echo '    include ("\${_prefix}/rdkit/rdkit-targets.cmake")'
#echo "--- change lib to rdkit in the middle of path name ---"

##CMake Error at /usr/local/lib/cmake/rdkit/rdkit-config.cmake:6 (include):
##  include could not find load file:
##
##    /usr/local/lib/cmake/lib/rdkit-targets.cmake
