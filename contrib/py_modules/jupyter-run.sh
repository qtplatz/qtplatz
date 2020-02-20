#!/bin/bash

__arch=`uname`

PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
PYTHON_VERSION_MAJOR=$(python3 -c "import sys; print( sys.version_info[0] )")
PYTHON_VERSION_MINOR=$(python3 -c "import sys; print( sys.version_info[1] )")

QTPLATZ_DIR=

#jupyter notebook --generate-config
#jupyter notebook password

echo "PYTHONPATH=" $PYTHONPATH
echo "PYTHON_INCLUDE=" $PYTHON_INCLUDE
echo "PYTHON_VERSION=" ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}

__dirs=()

case "${__arch}" in
	Linux*)
		__dirs+=("${PYTHON_INCLUDE}/dist-packages")
		;;
	Darwin*)
	    local home=~
		__dirs+=("$home/Desktop/qtplatz.app/Library/Python/${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages")
	    ;;
	*)
	    echo "######## unknown arch: " $__arch
	    ;;
esac

for dir in "${__dirs[@]}"; do
	if [ -d $dir/qtplatz ]; then
		QTPLATZ_DIR=$dir
	fi
done

export PYTHONPATH=$QTPLATZ_DIR/Library/Python/3.7/site-packages:$PYTHONPATH
jupyter notebook

#jupyter notebook --no-browser
