#!/bin/bash

cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/config.sh
source ${cwd}/prompt.sh
source ${cwd}/nproc.sh
SUDO=
cmake_args=()
PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
PYTHON_ROOT=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"data\"])")
PYTHON=$(python3 -c "import sys; print(sys.executable)")
NUMPY_INCLUDE=$(python3 -c "import numpy; print(numpy.get_include())")

arch=`uname`-`arch`
__nproc nproc

build_clean=false

while [ $# -gt 0 ]; do
	case "$1" in
		clean)
			shift
			build_clean=true
			;;
	esac
done

if [ -z $cross_target ]; then
	SUDO=sudo
    BUILD_DIR=$SRC/build-${arch}/rdkit.release
    RDBASE=${SRC}/rdkit
else
	SUDO=
    BUILD_DIR=${BUILD_ROOT}/rdkit.release
    RDBASE=${SRC}/rdkit
    if [ -z ${BOOST_ROOT} ]; then
		echo "********************* No BOOST_ROOT specified *****************"
		exit 1
    fi
fi

if [ $build_clean = true ]; then
	set -x
	rm -rf $BUILD_DIR
	exit
fi

if [ -z $cross_target ]; then
	if [ `uname` == "Darwin" ]; then
		cmake_args+=( "-DBOOST_ROOT=${BOOST_ROOT}"
					  "-DRDK_BUILD_INCHI_SUPPORT=ON"
					  "-DRDK_BUILD_PYTHON_WRAPPERS=ON"
					  "-DPYTHON_EXECUTABLE=${PYTHON}"
					  "-DPYTHON_INCLUDE_DIR=${PYTHON_INCLUDE}"
					  "-DPYTHON_NUMPY_INCLUDE_PATH=${NUMPY_INCLUDE}"
					  "-DRDK_INSTALL_INTREE=OFF"
					  "-DRDK_INSTALL_STATIC_LIBS=OFF"
					  "-DBoost_NO_BOOST_CMAKE=ON"
					  "-DRDK_BUILD_FREETYPE_SUPPORT=OFF"
					  "-DRDK_BUILD_RPATH_SUPPORT=ON"
					  "-DCMAKE_OSX_DEPLOYMENT_TARGET=12.0"
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
		cmake_args+=( "-DBOOST_ROOT=$BOOST_ROOT"
					  "-DRDK_BUILD_INCHI_SUPPORT=ON"
					  "-DRDK_BUILD_PYTHON_WRAPPERS=ON"
					  "-DPYTHON_EXECUTABLE=${PYTHON}"
					  "-DPYTHON_INCLUDE_DIR=${PYTHON_INCLUDE}"
					  "-DRDK_INSTALL_INTREE=OFF"
					  #"-DRDK_INSTALL_STATIC_LIBS=OFF"
					  #"-DRDK_INSTALL_DYNAMIC_LIBS=ON"
					  "-DBoost_NO_BOOST_CMAKE=ON"
					  "-DRDK_BUILD_FREETYPE_SUPPORT=OFF"
					)
	fi
else
	# cross build
    cmake_args+=( "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN" )
	cmake_args+=( "-DRDK_BUILD_INCHI_SUPPORT=ON"
				  "-DRDK_BUILD_PYTHON_WRAPPERS=OFF"
				  "-DRDK_INSTALL_INTREE=OFF"
				  "-DRDK_INSTALL_STATIC_LIBS=OFF"
				  "-DRDK_BUILD_FREETYPE_SUPPORT=OFF"
				)
fi

if [ ! -d ${RDBASE} ]; then
    if [ ! -d $(dirname $RDBASE) ]; then
		mkdir -p $(dirname $RDBASE)
    fi
	if [ -d ${BUILD_DIR} ]; then
	   # force clean destination if exists
	   set -x
	   rm -rf ${BUILD_DIR}
	fi
    git clone https://github.com/rdkit/rdkit ${RDBASE}
fi

if [ ! -d ${BUILD_DIR} ]; then
	mkdir -p ${BUILD_DIR};
fi

cd ${BUILD_DIR}
echo "RDBASE    : " ${RDBASE}
echo "BUILD_DIR : " `pwd`
echo cmake "${cmake_args[@]}" ${RDBASE}
prompt
cmake "${cmake_args[@]}" ${RDBASE}
echo "make -j${nproc}"
prompt
make -k -j${nproc}

if [ $? -eq 0 ]; then
	#make test
	echo ${SUDO} make install
	prompt
	${SUDO} make install
fi

#echo "Edit /usr/local/lib/cmake/rdkit/rdkit-config.cmake as:"
#echo '    include ("\${_prefix}/rdkit/rdkit-targets.cmake")'
#echo "--- change lib to rdkit in the middle of path name ---"

##CMake Error at /usr/local/lib/cmake/rdkit/rdkit-config.cmake:6 (include):
##  include could not find load file:
##
##    /usr/local/lib/cmake/lib/rdkit-targets.cmake
