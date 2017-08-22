#!/bin/bash

arch=`uname`-`arch`
cwd=`pwd`
source_dir=`pwd`
host_system=`uname`

build_target=$(basename $cwd)
build_root="../.."
config=release
build_clean=false
cmake_args=('-DCMAKE_BUILD_TYPE=Release' '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON')
source_dirs=("$cwd")
qtplatz_source_dir="$cwd/.."

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
	codelite)
	    config=debug
	    ide=codelite
	    shift
	    ;;
	xcode)
	    config=debug
	    ide=xcode
	    shift
	    ;;	    
	
	clean)
	    build_clean=true
	    shift
	    ;;
	*)
	    break
	    ;;
    esac
done

echo "platform=" $host_system
echo "config=" $config

if [ -z $cross_target ]; then
    build_dirs=("$build_root/build-$arch/$build_target.$config" )
    qtplatz_build_dir=$build_root/build-$arch/qtplatz.$config
    case $arch in
	Darwin-*)
	    if [ "$config" = "debug" ]; then
		if [ "$ide" = "xcode" ]; then
		    cmake_args=('-G' 'Xcode' '-DCMAKE_BUILD_TYPE=Debug')
		else
		    cmake_args=('-DCMAKE_BUILD_TYPE=Debug')
		fi
	    fi
	    ;;
	*)
	    
	    if [ $config = debug ]; then
		if [ $ide = eclipse ]; then
		    cmake_args=('-G' 'Eclipse CDT4 - Unix Makefiles' '-DCMAKE_ECLIPSE_VERSION=4.5' '-DCMAKE_BUILD_TYPE=Debug' '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON')
		elif [ $ide = codelite ]; then
		    cmake_args=('-G' 'CodeLite - Unix Makefiles' '-DCMAKE_BUILD_TYPE=Debug' '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON')
		else
		    cmake_args=('-G' 'CodeBlocks - Unix Makefiles' '-DCMAKE_BUILD_TYPE=Debug' '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON')
		fi
	    fi
	;;
    esac
else
    build_dirs=("$build_root/build-$cross_target/$build_target.$config" )
    qtplatz_build_dir=$build_root/build-$cross_target/qtplatz.$config
fi

echo "build_dirs: ${build_dirs[*]}"
echo "qtplatz build dir: " $qtplatz_build_dir

## Clean destination
if [ $build_clean = true ]; then
    for build_dir in ${build_dirs[@]}; do
	echo rm -rf $build_dir; rm -rf $build_dir
    done
    exit
fi

index=0
for build_dir in ${build_dirs[@]}; do

    source_dir=${source_dirs[$index]}

    echo "------------------------"
    echo "build for '$source_dir' --> '$build_dir'"
    echo "------------------------"    
    
    echo "#" mkdir -p $build_dir
    echo "#" cd $build_dir
    mkdir -p $build_dir
    cd $build_dir
    echo "#" pwd `pwd`

    if [ -z $cross_target ]; then
	echo "## Native build for $arch"

	echo cmake "${cmake_args[@]}" -D"QTPLATZ_BUILD_DIR=$qtplatz_build_dir" $source_dir
	cmake "${cmake_args[@]}" -D"QTPLATZ_BUILD_DIR=$qtplatz_build_dir" $source_dir
	
    else
	echo "## Cross build for $arch"
	case $cross_target in
	    helio|armv7l|de0-nano-soc|arm-linux-gnueabihf)
		toolchain_file=$qtplatz_source_dir/toolchain-arm-linux-gnueabihf.cmake
		cmake -DCMAKE_TOOLCHAIN_FILE=$toolchain_file -DQTPLATZ_CORELIB_ONLY=1 $source_dir
		;;
	    raspi)
		toolchain_file=$qtplatz_source_dir/toolchain-raspi.cmake
		cmake -DCMAKE_TOOLCHAIN_FILE=$toolchain_file -DCMAKE_PREFIX_PATH=/opt/qt5pi $source_dir
		;;
	    *)
		echo "Unknown cross_target: $cross_target"
		;;
	esac    
    fi
    cd $cwd
    index=$((index+1))
done



