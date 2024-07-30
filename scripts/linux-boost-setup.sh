#!/bin/bash

cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/config.sh
source ${cwd}/prompt.sh
source ${cwd}/nproc.sh
bjam_args=()

build_clean=false

while [ $# -gt 0 ]; do
	case "$1" in
		clean)
			shift
			build_clean=true
			;;
	esac
done

function boost_download {
    BOOST_VERSION=$1
    BUILD_ROOT=$2
    BOOST_BUILD_DIR=$3

    if [ ! -f ${DOWNLOADS}/boost-${BOOST_VERSION}.tar.bz2 ]; then
		echo "=============================="
		VERSION=$(echo $BOOST_VERSION | tr _ .)
		echo curl -L -o ${DOWNLOADS}/boost-${BOOST_VERSION}.tar.bz2 \
			 https://boostorg.jfrog.io/artifactory/main/release/${VERSION}/source/boost_${BOOST_VERSION}.tar.bz2
		curl -L -o ${DOWNLOADS}/boost-${BOOST_VERSION}.tar.bz2 \
			 https://boostorg.jfrog.io/artifactory/main/release/${VERSION}/source/boost_${BOOST_VERSION}.tar.bz2
	fi

	if [ ! -d ${BOOST_BUILD_DIR} ]; then
		echo "BOOST_BUID_DIR=" ${BOOST_BUILD_DIR} " does not exists -- may i create?"
		prompt
		tar xvf ${DOWNLOADS}/boost-${BOOST_VERSION}.tar.bz2 -C $(dirname ${BOOST_BUILD_DIR})
	fi
}

function zlib_download {
	if [ ! -f ${DOWNLOADS}/zlib-1.3.tar.gz ]; then
		curl -L -o ${DOWNLOADS}/zlib-1.3.tar.gz https://zlib.net/zlib-1.3.tar.gz
	fi
	if [ ! -d ${BUILD_ROOT}/zlib-1.3 ]; then
		tar xvf ${DOWNLOADS}/zlib-1.3.tar.gz -C ${BUILD_ROOT}
	fi
}

function bzip2_download {
	if [ ! -f ${DOWNLOADS}/bzip2-latest.tar.gz ]; then
		curl -L -o ${DOWNLOADS}/bzip2-latest.tar.gz https://sourceware.org/pub/bzip2/bzip2-latest.tar.gz
	fi
	if [ ! -d ${BUILD_ROOT}/bzip2-1.0.8.tar.gz ]; then
		tar xvf ${DOWNLOADS}/bzip2-1.0.8.tar.gz -C ${BUILD_ROOT}
	fi
}


function python_dirs {
	PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
	PYTHON_ROOT=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"data\"])")
	PYTHON=$(python3 -c "import sys; print(sys.executable)")
	PYTHON_VERSION=$(python3 -c "import sys; print('{}.{}'.format(*sys.version_info))")
}

function make_user_config_darwin {
	PYTHON=$(python3 -c "import sys; print(sys.executable)")
	PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
	PYTHON_ROOT=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"data\"])")
	if [ -f ~/user-config.jam ]; then
		mv ~/user-config.jam ~/user-config.jam.orig
	fi

	cat << END > ~/user-config.jam
using python
	  : $PYTHON_VERSION
	  : $PYTHON
	  : $PYTHON_INCLUDE
	  ;
END
}
# using clang : : : <cxxflags>"-mmacosx-version-min=10.15" ;

function make_user_config_linux {
	PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
	PYTHON_ROOT=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"data\"])")
	PYTHON=$(python3 -c "import sys; print(sys.executable)")

	if [ -f ~/user-config.jam ]; then
		mv ~/user-config.jam ~/user-config.jam.orig
	fi

	if [ ! -f ~/user-config.jam ]; then
	cat << END > ~/user-config.jam
using python
	  : $PYTHON_VERSION
	  : $PYTHON
	  : $PYTHON_INCLUDE
	  ;
END
	fi
}

function make_user_config_mingw {
	PYTHON=$(which python3)
	PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
	PYTHON_ROOT=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"data\"])")
	#	PYTHON=$(python3 -c "import sys; print(sys.executable)")

	if [ -f ~/user-config.jam ]; then
		mv ~/user-config.jam ~/user-config.jam.orig
	fi

	if [ ! -f ~/user-config.jam ]; then
	cat << END > ~/user-config.jam
using python
	  : $PYTHON_VERSION
	  : $PYTHON
	  : $PYTHON_INCLUDE
	  ;
END
	fi
}

function boost_build_mingw {
	PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
	PYTHON_ROOT=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"data\"])")
	PYTHON=$(python3 -c "import sys; print(sys.executable)")
	CXX_FLAGS="-fPIC -std=c++17"
	C_FLAGS="-fPIC"
	SUDO=
	echo ./bootstrap.sh --prefix=${BOOST_INSTALL_PREFIX} --with-python=${PYTHON}
	echo ./b2 -j $nproc address-model=64 toolset=gcc cflags="${C_FLAGS}" cxxflags="${CXX_FLAGS}" \
		 threading=multi \
		 link=shared \
		 include="${PYTHON_INCLUDE}" \
		 hardcode-dll-paths=true \
		 dll-path="'\$ORIGIN/../lib'" \
		 --without-mpi \
		 --without-graph_parallel \
		 install
	prompt
	./bootstrap.sh --prefix=${BOOST_INSTALL_PREFIX} --with-python=${PYTHON} &&
		./b2 -j $nproc address-model=64 toolset=gcc cflags="${C_FLAGS}" cxxflags="${CXX_FLAGS}" \
			 threading=multi \
			 link=shared \
			 include="${PYTHON_INCLUDE}" \
			 hardcode-dll-paths=true \
			 dll-path="'\$ORIGIN/../lib'" \
			 --without-mpi \
			 --without-graph_parallel \
			 install
}


function boost_build {
    echo "=============================="
    echo "   BOOST install for $(build_uname) "
    echo "=============================="

    BOOST_BUILD_DIR=$1

    ( cd ${BOOST_BUILD_DIR};
      echo "---------> boost build in $(pwd)"

      case "$(build_uname)" in
		  mingw*)
			  boost_build_mingw
			  ;;
		  Linux*)
			  release=$(lsb_release -sr)
			  if [ "${release%.*}" -lt "10" ]; then
				  echo "********************************************"
				  echo "If libbz2.a exists in the sytem library path on Debian 9, iostreams build may be failed."
				  echo "********************************************"
			  fi
			  PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
			  PYTHON_ROOT=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"data\"])")
			  PYTHON=$(python3 -c "import sys; print(sys.executable)")
			  CXX_FLAGS="-fPIC -std=c++17"
			  C_FLAGS="-fPIC"
			  echo ./bootstrap.sh --prefix=$BOOST_INSTALL_PREFIX --with-python=${PYTHON}
			  echo ./b2 -j $nproc address-model=64 toolset=gcc cflags="${C_FLAGS}" cxxflags="${CXX_FLAGS}" \
					   threading=multi \
					   link=shared \
					   include="${PYTHON_INCLUDE}" \
					   hardcode-dll-paths=true \
					   dll-path="'\$ORIGIN/../lib'" \
					   --without-mpi \
					   --without-graph_parallel \
					   install
			  prompt
			  ./bootstrap.sh --prefix=$BOOST_INSTALL_PREFIX --with-python=${PYTHON} &&
			  	  sudo ./b2 -j $nproc address-model=64 toolset=gcc cflags="${C_FLAGS}" cxxflags="${CXX_FLAGS}" \
					   threading=multi \
					   link=shared \
					   include="${PYTHON_INCLUDE}" \
					   hardcode-dll-paths=true \
					   dll-path="'\$ORIGIN/../lib'" \
					   --without-mpi \
					   --without-graph_parallel \
					   install
			  ;;
		  Darwin*)
			  echo "***********************************************************************************************************"
			  echo "if you got failed by zlib, try following command."
			  echo "sudo installer -pkg /Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_10.14.pkg -target /"
			  echo "--- macosx-version-min= issue: edit the tools/build/v2/tools/darwin.jam file"
			  echo "feature.extend macosx-version-min : 4.15 ; <-- 'below feature macosx-version-min : : propagated optional ;' line"
			  echo "***********************************************************************************************************"
			  #PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
			  #PYTHON_ROOT=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"data\"])")
			  #PYTHON=$(python3 -c "import sys; print(sys.executable)")
			  #OSX_VERSION_MIN=-mmacosx-version-min=10.12
			  CXX_FLAGS="-std=c++17"
			  LINKFLAGS="-stdlib=libc++"
			  export BZIP2_SOURCE="${BUILD_ROOT}/bzip2-1.0.8"
			  echo "BZIP2_SOURCE=${BZIP2_SOURCE}"
			  #export ZLIB_SOURCE="${BUILD_ROOT}/zlib-1.3.1" <-- this will conflict w/ libz-1.2.12 (macOS native) at macdeployqt step
			  #echo "ZLIB_SOURCE=${ZLIB_SOURCE}"
			  echo ./bootstrap.sh --prefix=$BOOST_INSTALL_PREFIX --with-toolset=clang --with-python=${PYTHON} \
				   --with-python-root=${PYTHON_ROOT} --with-python-version=${PYTHON_VERSION}
			  echo ./b2 -j $nproc address-model=64 cxxflags="-std=c++20" toolset=clang linkflags="$LINKFLAGS" include=${PYTHON_INCLUDE}
			  prompt
			  ./bootstrap.sh --prefix=$BOOST_INSTALL_PREFIX --with-toolset=clang --with-python=${PYTHON} \
							 --with-python-root=${PYTHON_ROOT} --with-python-version=${PYTHON_VERSION}
			  ./b2 -j $nproc address-model=64 cxxflags="-std=c++20" toolset=clang linkflags="$LINKFLAGS" include=${PYTHON_INCLUDE}
			  sudo ./b2 install
			  ;;
		  *)
			  echo "Unknown build: " $(build_uname)
      esac
    )
}

function boost_cross_build {
    echo "=============================="
    echo "   BOOST cross install for $cross_target "
    echo "=============================="
    BOOST_BUILD_DIR=$1
    if [ ! -d $(dirname ${BOOST_INSTALL_PREFIX}) ]; then
		if ! mkdir -p $(dirname $BOOST_INSTALL_PREFIX) ; then
			echo "mkdir -p $(dirname BOOST_INSTALL_PREFIX) -- command faild. Check for your access permission"
			exit
		fi
    fi

    if [ ! -w $(dirname $BOOST_INSTALL_PREFIX) ]; then
		echo "Make $(dirname $BOOST_INSTALL_PREFIX) writable."
    fi

	case ${cross_target} in
		arm-linux-gnueabihf|armhf|armv7l|de0-nano-soc|helio)
			toolset=gcc-arm
			cat << END > ${BOOST_BUILD_DIR}/user-config.jam
using gcc :	arm : arm-linux-gnueabihf-g++ : <cxxflags>"-std=c++17 -fPIC" ;
using python : 2.7 ;
END
			;;
		x86_64-w64-mingw32)
			zlib_download
			bzip2_download
			bjam_args+=( 'address-model=64' 'architecture=x86' )
			bjam_args+=( 'threading=multi' 'link=shared' 'hardcode-dll-paths=true' )
			bjam_args+=( dll-path="'\$ORIGIN/../lib'" '--without-mpi' '--without-graph_parallel' )
			#bjam_args+=( -sBZIP2_SOURCE="${BUILD_ROOT}/bzip2-1.0.8" )
			#bjam_args+=( -sZLIB_SOURCE="${BUILD_ROOT}/zlib-1.3" )

			cat << END > ${BOOST_BUILD_DIR}/user-config.jam
using gcc  :	: x86_64-w64-mingw32-g++ : <cxxflags>"-std=c++17 -fPIC" ;
using zlib :	:
	  <include>/usr/x86_64-w64-mingw32/include
	  <search>/usr/x86_64-w64-mingw32/lib
	  ;
using bzip2 :	:
	    <source>"${BUILD_ROOT}/bzip2-1.0.8"
	  ;
END
			;;
		*)
			echo "-------- unknown target ---------- ${cross_target}"
			;;
	esac

    ( cd $BOOST_BUILD_DIR;
      echo $(pwd)
      echo ./bootstrap.sh --prefix=${BOOST_INSTALL_PREFIX}
      echo ./b2 --user-config=./user-config.jam "${bjam_args[@]}" -j${nproc} install
      prompt
      ./bootstrap.sh --prefix=${BOOST_INSTALL_PREFIX}
      ./b2 --user-config=./user-config.jam "${bjam_args[@]}" -j${nproc} install
    )
}

if [ -z $PREFIX ]; then
    PREFIX=/usr/local
fi

python_dirs

BOOST_BUILD_DIR=${BUILD_ROOT}/boost_${BOOST_VERSION}

if [ ! -z ${cross_target} ]; then
	BOOST_INSTALL_PREFIX="${CROSS_ROOT}${BOOST_INSTALL_PREFIX}"
fi

if [ $build_clean = true ]; then
	set -x
	rm -rf $BOOST_BUILD_DIR
	exit
fi

echo "INSTALLING 'boost' $cross_target to:"
echo "	\${DOWNLOAD}             : ${DOWNLOADS}"
echo "	\${BOOST_VERSION}        : ${BOOST_VERSION}"
echo "	\${BOOST_ROOT}           : ${BOOST_ROOT}"
echo "	\${BOOST_INSTALL_PREFIX} : ${BOOST_INSTALL_PREFIX}"
echo "	\${BUILD_ROOT}           : ${BUILD_ROOT}"
if [ ! -z ${cross_target} ]; then echo "	\${CROSS_ROOT}           : ${CROSS_ROOT}"; fi
echo "	BOOST_BUILD_DIR         : ${BOOST_BUILD_DIR}"
echo "	PYTHON                  : ${PYTHON}"
echo "	PYTHON_VERSION          : ${PYTHON_VERSION}"
echo "	PYTHON_INCLUDE          : ${PYTHON_INCLUDE}"
echo "	PYTHON_ROOT             : ${PYTHON_ROOT}"

if [ -z $cross_target ]; then
	case "`build_uname`" in
		Darwin*)
			make_user_config_darwin
			;;
		Linux*)
			make_user_config_linux
			;;
		mingw*)
			make_user_config_mingw
			;;
		*)
			echo "-------- unknown target ---------- `build_uname`"
			;;
	esac
fi

if [ ! -d ${BOOST_BUILD_DIR} ]; then
    echo "	BOOST_BUILD_DIR   : download to ${BOOST_BUILD_DIR}"
else
    echo "	BOOST_BUILD_DIR   : ${BOOST_BUILD_DIR}"
fi

if [ -d ${BOOST_INSTALL_PREFIX} ]; then
    echo "boost-${BOOST_VERSION} already installed in ${BOOST_INSTALL_PREFIX}"
fi

__nproc nproc

if [ ! -d ${BUILD_ROOT} ]; then
    mkdir -p ${BUILD_ROOT}
fi

boost_download ${BOOST_VERSION} ${BUILD_ROOT} ${BOOST_BUILD_DIR}

if [ -z $cross_target ]; then

	#	if [ -f ~/user-config.jam ]; then
	#		mv ~/user-config.jam ~/user-config.jam.orig
	#	fi
	echo "boost_build ${BOOST_BUILD_DIR}"
	prompt
    boost_build ${BOOST_BUILD_DIR}

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
			echo sudo usermod -aG staff $USER
			sudo usermod -aG staff $USER
		fi
		if [ ! -w $CROSS_ROOT ]; then
			echo "You may need to logout/login cycle"
			exit 1
		fi
    fi
	if [ ! -d ${BOOST_INSTALL_PREFIX} ]; then
		mkdir -p ${BOOST_INSTALL_PREFIX}
	fi

	boost_cross_build ${BOOST_BUILD_DIR}
fi

echo "=============================="
echo "   BOOST $BOOST_VERSION Installed "
echo "=============================="
