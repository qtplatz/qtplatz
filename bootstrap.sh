#!/bin/bash

arch=`uname`-`arch`
cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
config=release
source_dir=`pwd`
host_system=`uname`
build_clean=false
build_package=false
build_root=..
cmake_args=('-DCMAKE_BUILD_TYPE=Release')

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
	codelite)
	    config=debug	    
	    ide=codelite
	    shift
	    ;;	
	xcode)
	    ide=xcode
	    shift
	    ;;
	vscode)
	    ide=vscode
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

if [ -z $cross_target ]; then
    case $arch in
	Darwin-*)
	    source_dirs=("$cwd")
	    build_dirs=( "$build_root/build-$arch/qtplatz.$config" )
	    if [ "$config" = "debug" ]; then
			if [ "$ide" = "xcode" ]; then
		    cmake_args=('-G' 'Xcode' '-DCMAKE_BUILD_TYPE=Debug')
			else
				cmake_args=('-DCMAKE_BUILD_TYPE=Debug')
			fi
	    fi
	    ;;
	*)
	    source_dirs=( "$cwd" )
	    build_dirs=( "$build_root/build-$arch/qtplatz.$config" )	    
	    if [ "$config" = "debug" ]; then
			cmake_args=('-DCMAKE_BUILD_TYPE=Debug')
			if [ "$ide" = "eclipse" ]; then
				cmake_args=('-G' 'Eclipse CDT4 - Unix Makefiles' '-DCMAKE_ECLIPSE_VERSION=4.5' '-DCMAKE_BUILD_TYPE=Debug')
			fi
			if [ "$ide" = "codelite" ]; then
				cmake_args=('-G' 'CodeLite - Unix Makefiles' '-DCMAKE_BUILD_TYPE=Debug' '-DCMAKE_EXPORT_COMPILE_COMMANDS=ON')
			fi
	    fi
	    ;;
    esac
else
    echo "cross_target="$cross_target
    source_dirs=( "$cwd" )
    build_dirs=( "$build_root/build-$cross_target/qtplatz.$config" )
fi

## Clean destinatiuon 
if [ $build_clean = true ]; then
    for build_dir in ${build_dirs[@]}; do
	echo rm -rf $build_dir; rm -rf $build_dir
    done
    exit
fi

if [ -z "$QTDIR" ]; then
    if find_qmake QMAKE; then
	QTDIR=$($QMAKE -query QT_HOST_PREFIX); export QTDIR
	echo "$0: qmake found in "$QTDIR " (qmake="${QMAKE}")"
	prompt
	export PATH=$QTDIR/bin:$PATH
    else
	echo "$0: ## Error: QMAKE cannot be found"
	exit 1
    fi
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

	echo cmake "${cmake_args[@]}" $source_dir
	cmake "${cmake_args[@]}" $source_dir

    else
	echo "#######################################"
	echo "## Cross build for $cross_target"
	echo "#######################################"	
	case $cross_target in
	    armhf|armv7l|arm-linux-gnueabihf|de0-nano-soc|helio)
		toolchain_file=$cwd/toolchain-arm-linux-gnueabihf.cmake
		cmake -DCMAKE_TOOLCHAIN_FILE=$toolchain_file \
		      -DQTPLATZ_CORELIB_ONLY=ON -DWITH_QT5=OFF -DWITH_QWT=OFF -DWITH_OPENCV=OFF -DWITH_RDKIT=OFF $source_dir
		echo cp $toolchain_file $build_dir/toolchain.cmake
		cp $toolchain_file toolchain.cmake 
		;;
	    raspi)
		toolchain_file=$cwd/toolchain-raspi.cmake
		cmake -DCMAKE_TOOLCHAIN_FILE=$toolchain_file -DCMAKE_PREFIX_PATH=/opt/qt5pi $source_dir
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

cat > ${build_dirs[0]}/../qtplatz-build.sh <<EOF
#!/bin/bash
for i in ${build_dirs[*]}; do (cd \$i; cmake . ; make -j8 package); done
for i in ${build_dirs[*]}; do (cd \$i; mv *.deb ..); done
EOF

chmod +x ${build_dirs[0]}/../qtplatz-build.sh
