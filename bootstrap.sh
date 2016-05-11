#!/bin/bash

arch=`arch`
cwd=`pwd`
config=release
source_dir=`pwd`
host_system=`uname`
build_clean=false
build_package=false
build_root=..

while [ $# -gt 0 ]; do
    case "$1" in
	debug|eclipse)
	    config=debug
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

source_dirs=("$cwd" "$cwd/contrib/installer/boost")

if [ -z $cross_target ]; then
    build_dirs=("$build_root/build-$arch/qtplatz.$config" \
		    "$build_root/build-$arch/boost" )
else
    build_dirs=("$build_root/build-$cross_target/qtplatz.$config" \
		    "$build_root/build-$cross_target/boost")
fi

## Clean destinatiuon 
if [ $build_clean = true ]; then
    for build_dir in ${build_dirs[@]}; do
	echo rm -rf $build_dir; rm -rf $build_dir
    done
    exit
fi

echo "build_dirs: ${build_dirs[*]}"

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
	case $arch in
	    Darwin)	    
		if [ $config=debug ]; then
		    cmake -G Xcode -DCMAKE_BUILD_TYPE=Debug $source_dir
		else
		    echo `pwd`
		    cmake -DCMAKE_BUILD_TYPE=Release $source_dir
		fi
		;;
	    *)
		if [ $config=debug ]; then
		    cmake -G "Eclipse CDT4 - Unix Makefiles" \
			  -DCMAKE_ECLIPSE_VERSION=4.5 \
			  -DCMAKE_BUILD_TYPE=Debug $source_dir
		else
		    echo `pwd`
		    cmake -DCMAKE_BUILD_TYPE=Release $source_dir
		    fi
		;;
	esac
    else
	echo "## Cross build for $arch"
	case $cross_target in
	    helio|armv7l|de0-nano-soc|arm-linux-gnueabihf)
		toolchain_file=$cwd/toolchain-arm-linux-gnueabihf.cmake
		cmake -DCMAKE_TOOLCHAIN_FILE=$toolchain_file \
		      -DQTPLATZ_CORELIB_ONLY=1 $source_dir
		cp $toolchain_file $build_dir/toolchain.cmake
		;;
	    raspi)
		toolchain_file=$cwd/toolchain-raspi.cmake
		cmake -DCMAKE_TOOLCHAIN_FILE=$toolchain_file -DCMAKE_PREFIX_PATH=/opt/qt5pi $source_dir
		cp $toolchain_file $build_dir/toolchain.cmake
		;;
	    *)
		echo "Unknown cross_target: $cross_target"
		;;
	esac    
    fi
    cd $cwd
    index=$((index+1))
done

cat > ${build_dirs[0]}/../qtplatz-build.sh <<EOF
#!/bin/bash
for i in ${build_dirs[*]}; do (cd \$i; make -j4 package); done
for i in ${build_dirs[*]}; do (cd \$i; mv *.deb ..); done
EOF

chmod +x ${build_dirs[0]}/../qtplatz-build.sh
