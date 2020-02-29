#!/bin/bash

os=`uname`
arch=`uname`-`arch`
config=
cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
PYTHON_VERSION_MAJOR=$(python3 -c "import sys; print( sys.version_info[0] )")
PYTHON_VERSION_MINOR=$(python3 -c "import sys; print( sys.version_info[1] )")

QTPLATZ_DIR=
RDKIT_DIR=
__PATH=

while [ $# -gt 0 ]; do
    case "$1" in
	debug)
	    config=debug
	    shift
	    ;;
	run)
		run=true
		shift
		;;
	python3)
		python3=true
		shift
		;;	
	*)
	    echo "unknown option $1"
	    exit 1
	    ;;
    esac
done

#jupyter notebook --generate-config
#jupyter notebook password

echo "PYTHON_VERSION=" ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}
echo "config: " $config

case "${arch}" in
	Linux*)
	    if [ "$config" = "debug" ]; then
			__search_path=("${HOME}/src/build-${arch}/qtplatz.release/python${PYTHON_VERSION}" )
		fi
		__search_path+=("/usr/local/lib/python${PYTHON_VERSION}/dist-packages"
						"/usr/local/lib/python${PYTHON_VERSION_MAJOR}/dist-packages" ) # RDKit py modules on Linux
		;;
	Darwin*)
		if [ "$config" = "debug" ]; then
			__search_path=("${HOME}/src/build-${arch}/qtplatz.release/bin/qtplatz.app/Library/Python/${PYTHON_VERSION}/site-packages")
		fi
		__search_path+=("${HOME}/Desktop/qtplatz.app/Library/Python/${PYTHON_VERSION}/site-packages"
						"/Applications/qtplatz.app/Library/Python/${PYTHON_VERSION}/site-packages"
						"/usr/local/lib/python${PYTHON_VERSION}/site-packages") # RDKit py modules on mac 
		;;
	    ;;
	*)
	    echo "######## unknown arch: " $arch
		exit 1
	    ;;
esac

for dir in "${__search_path[@]}"; do
	if [ -d $dir/qtplatz ]; then
		QTPLATZ_DIR=$dir
		break
	fi
done
for dir in "${__search_path[@]}"; do
	if [ -d $dir/rdkit ]; then
		RDKIT_DIR=$dir
		break
	fi
done

export PYTHONPATH=${QTPLATZ_DIR}:${RDKIT_DIR}
echo "PYTHONPATH=" $PYTHONPATH

#jupyter notebook
if [ ${run} == true ]; then
	jupyter notebook --no-browser
fi
if [ ${python3} == true ]; then
	python3
fi
