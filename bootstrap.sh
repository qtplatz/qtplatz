#!/bin/bash

arch=`uname`-`arch`
cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
config=release
source_dir=`pwd`
host_system=`uname`
build_clean=false
build_package=false
build_root=..
cmake_args=()

source ${cwd}/scripts/find_qmake.sh
source ${cwd}/scripts/prompt.sh

while [ $# -gt 0 ]; do
    case "$1" in
	debug)
	    config=debug
	    shift
	    ;;
	eclipse)
	    config=debug
	    ide=eclipse
	    shift
	    ;;
	xcode)
	    ide=xcode
	    shift
	    ;;
	ninja)
		ide=ninja
		shift
		;;
	clean)
	    build_clean=true
	    shift
	    ;;
	*)
	    echo "unknown option $1"
	    exit 1
	    ;;
    esac
done

echo -e "$0: platform=\t"$host_system
echo -e "$0: config  =\t"$config

if [ "$config" = "debug" ]; then
	cmake_args+=('-DCMAKE_BUILD_TYPE=Debug')
else
	cmake_args+=('-DCMAKE_BUILD_TYPE=Release')
fi

if [ -z "$cross_target" ]; then
	if [ -z "$QTDIR" ]; then
		if ! find_qmake QMAKE; then
			echo "##### find_qmake failed."
			echo "##### set QTDIR environment variable, e.g. QTDIR=/opt/Qt/6.5.3/macos"
			exit 1;
		fi
	elif [ -f "${QTDIR}/bin/qmake" ]; then
		QMAKE="${QTDIR}/bin/qmake"
	fi
	if [ ! -z ${QMAKE} ]; then
		if ${QMAKE} --version &> /dev/null; then
			QTDIR=$($QMAKE -query QT_HOST_PREFIX)
			cmake_args+=("-DCMAKE_PREFIX_PATH=${QTDIR}")
			QT_VERSION=$($QMAKE -query QT_VERSION)
			if [[ ${QT_VERSION} =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)$ ]]; then
				QT_VERSION_MAJOR=${BASH_REMATCH[1]}
				QT_VERSION_MINOR=${BASH_REMATCH[2]}
				QT_VERSION_PATCH=${BASH_REMATCH[3]}
			fi
		else
			echo "QMAKE NOT Found."
			exit 1
		fi
	fi
fi

if [ "$ide" = "xcode" ]; then
	cmake_args+=('-G' 'Xcode')
elif [ "$ide" = "eclipse" ]; then
	cmake_args+=('-G' 'Eclipse CDT4 - Unix Makefiles' '-DCMAKE_ECLIPSE_VERSION=4.5')
elif [ "$ide" = "ninja" ]; then
	cmake_args+=('-G' 'Ninja')
fi

if [ -z $cross_target ]; then
	source_dirs=("$cwd")
	build_dirs=( "$build_root/build-$arch/qtplatz-qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${config}" )
else
    echo "cross_target=${cross_target}"
    source_dirs=( "$cwd" )
    build_dirs=( "$build_root/build-$cross_target/qtplatz.${config}" )
fi

## Clean destinatiuon
if [ $build_clean = true ]; then
    for build_dir in ${build_dirs[@]}; do
	echo rm -rf $build_dir; rm -rf $build_dir
    done
    exit
fi

echo "------------------------"
echo "-- build_dirs: ${build_dirs[*]}"
echo "-- QMAKE found in ${QMAKE} -- QT_VERSION: ${QT_VERSION} -- Qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}"

index=0
for build_dir in ${build_dirs[@]}; do

    source_dir=${source_dirs[$index]}

    echo "build for '$source_dir' --> '$build_dir'"
    echo "------------------------"

    echo "#" mkdir -p $build_dir
    echo "#" cd $build_dir
    mkdir -p $build_dir
    cd $build_dir
    echo "#" pwd `pwd`

    if [ -z $cross_target ]; then
		echo ""
		echo cmake "${cmake_args[@]}" $source_dir
		echo ""
		prompt
		cmake "${cmake_args[@]}" $source_dir
    else
		echo "#######################################"
		echo "## Cross build for $cross_target"
		echo "#######################################"
		case $cross_target in
			armhf|armv7l|arm-linux-gnueabihf|de0-nano-soc|helio)
				toolchain_file=$cwd/toolchain-arm-linux-gnueabihf.cmake
				cmake -DCMAKE_TOOLCHAIN_FILE=$toolchain_file \
					  -DWITH_QWT=OFF \
					  -DWITH_OPENCV=OFF \
					  -DWITH_RDKIT=OFF \
					  $source_dir
				echo cp $toolchain_file $build_dir/toolchain.cmake
				cp $toolchain_file toolchain.cmake
				;;
			raspi)
				toolchain_file=$cwd/toolchain-raspi.cmake
				cmake -DCMAKE_TOOLCHAIN_FILE=$toolchain_file -DCMAKE_PREFIX_PATH=/opt/qt5pi $source_dir
				cp $toolchain_file toolchain.cmake
				;;
			x86_64-w64-mingw32|msys2|mingw32)
				toolchain_file=$cwd/toolchain-x86_64-w64-mingw32.cmake
				cmake -DCMAKE_TOOLCHAIN_FILE=$toolchain_file $source_dir
				cp $toolchain_file toolchain.cmake
				;;
			*)
				echo "Unknown cross_target: $cross_target"
				;;
		esac
    fi
    cd $cwd
    index=$((index+1))
done
