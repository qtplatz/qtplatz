#!/bin/bash

__arch=`uname`
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

case "${__arch}" in
	Linux*)
		# for QtPlatz
	    if [ "$config" = "debug" ]; then
			__qtpath=("${HOME}/src/build-Linux-x86_64/qtplatz.release/python3.7")
		else
			__qtpath=("/usr/local/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/dist-packages")
		fi
		for dir in "${__qtpath[@]}"; do
			if [ -d $dir/qtplatz ]; then
				__PATH=$dir
			fi
		done
		# for RDKit
		__rdpath=("/usr/local/lib/python${PYTHON_VERSION_MAJOR}/dist-packages")
		for dir in "${__rdpath[@]}"; do
			if [ -d $dir/rdkit ]; then
				__PATH=$__PATH:$dir
			fi
		done
		;;
	Darwin*)
	    local home=~
	    if [ "$config" = "debug" ]; then
			__qtpath=("~/src/build-Darwin-i386/qtplatz.release")
		else
			__qtpath+=("$home/Desktop/qtplatz.app/Library/Python/${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages")
		fi
		for dir in "${__qtpath[@]}"; do
			if [ -d $dir/qtplatz ]; then
				__PATH=$dir
			fi
		done
	    ;;
	*)
	    echo "######## unknown arch: " $__arch
	    ;;
esac

export PYTHONPATH=$__PATH:$PYTHONPATH

echo "PYTHONPATH=" $PYTHONPATH
#jupyter notebook
jupyter notebook --no-browser
