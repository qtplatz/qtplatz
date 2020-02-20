#!/bin/bash

__arch=`uname`

PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
PYTHON_VERSION_MAJOR=$(python3 -c "import sys; print( sys.version_info[0] )")
PYTHON_VERSION_MINOR=$(python3 -c "import sys; print( sys.version_info[1] )")

QTPLATZ_DIR=
RDKIT_DIR=
__PATH=

#jupyter notebook --generate-config
#jupyter notebook password

echo "PYTHON_VERSION=" ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}

case "${__arch}" in
	Linux*)
		# for QtPlatz
		__qtpath=("/usr/local/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/dist-packages")
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
		__qtpath+=("$home/Desktop/qtplatz.app/Library/Python/${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages")
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
jupyter notebook

#jupyter notebook --no-browser
