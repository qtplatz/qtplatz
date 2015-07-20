#!/bin/sh

arch=`arch`
source_dir=`pwd`
build_debug=false
build_clean=false
build_package=false
build_root=..

if [ -z $cross_target ]; then
    cross_target=$arch
fi

while [ $# -gt 0 ]; do
    case "$1" in
	debug|eclipse)
	    echo "*********** building eclipse project"
	    build_debug=true
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

if [ $build_debug = true ]; then
    build_dir=$build_root/build/qtplatz-$cross_target.debug
else
    build_dir=$build_root/build/qtplatz-$cross_target.release
fi

if [ $build_clean = true ]; then
    echo "rm -rf $build_dir"; rm -rf $build_dir
    exit
fi

mkdir -p $build_dir
cd $build_dir

echo "#############"
echo "# creating build environment for qtplatz for target: $cross_target"
echo "# build_dir: `pwd`"

case $cross_target in
    helio)
	cmake -DCMAKE_TOOLCHAIN_FILE=$source_dir/toolchain-$cross_target.cmake \
	      -DCMAKE_PREFIX_PATH=/usr/local/qt5 -DQTPLATZ_CORELIB_ONLY=1 $source_dir
	;;
    raspi)
	cmake -DCMAKE_TOOLCHAIN_FILE=$source_dir/toolchain-raspi.cmake \
	      -DCMAKE_PREFIX_PATH=/opt/qt5pi $source_dir
	;;
    armv7l)
	cmake -DCMAKE_PREFIX_PATH=/usr/local/qt5 -DQTPLATZ_CORELIB_ONLY=1 $source_dir
	;;
    i686)
	cmake -DCMAKE_PREFIX_PATH=/opt/Qt/5.4/gcc $source_dir
	;;	
    x86_64)
	if [ $build_debug = true ]; then
	    cmake -G "Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug $source_dir
	else
	    echo `pwd`
	    cmake -DCMAKE_BUILD_TYPE=Release $source_dir
	fi
	;;
    *)
	echo "Unknown cross_target: $cross_target"
	;;
esac
