#!/bin/bash

os=`uname`
arch=`uname`-`arch`
config=
cwd="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
command="notebook"

PYTHON_INCLUDE=$(python3 -c "from sysconfig import get_paths as gp; print(gp()[\"include\"])")
PYTHON_VERSION_MAJOR=$(python3 -c "import sys; print( sys.version_info[0] )")
PYTHON_VERSION_MINOR=$(python3 -c "import sys; print( sys.version_info[1] )")
PYTHON_VERSION="${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}"

QTPLATZ_DIR=
RDKIT_DIR=
__search_path=()

while [ $# -gt 0 ]; do
    case "$1" in
	"debug" | "release" | "source" | "package" )
	    config="$1"
	    shift
	    ;;
	install)
		python3 -m pip install --upgrade pip
		python3 -m pip install notebook
		python3 -m pip install pillow
		shift
		;;
	python*)
		command="python3"
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
echo "config: " "$config"
ECHO "arch:   " "${arch}"

case "${arch}" in
	"Linux"*)
		echo "--------- found Linux ------------- config: " ${config}
		__search_path+=("/usr/local/lib/python${PYTHON_VERSION_MAJOR}/dist-packages" # RDKit
						"/usr/local/lib/python${PYTHON_VERSION}/dist-packages"       # QtPlatz installed
						"${HOME}/src/build-${arch}/qtplatz.${config}/python${PYTHON_VERSION}"
					   )
		;;
	"Darwin"*)
		echo "--------- found macOS ------------- config: " ${config}
		if [ "$config" == "package" ]; then
			__search_path=( "${HOME}/src/build-Darwin-i386/qtplatz.release/package/qtplatz.app/Library/Python/3.7/site-packages/" )
		elif [ "$config" == "source" ]; then			
			__search_path=( "${HOME}/src/build-Darwin-i386/qtplatz.release/qtplatz.app/Library/Python/3.7/site-packages/"
							"/usr/local/lib/python${PYTHON_VERSION}/site-packages"
						  )
		else
			__search_path+=( "/Volumes/qtplatz/qtplatz.app/Library/Python/${PYTHON_VERSION}/site-packages"
							 "${HOME}/Desktop/qtplatz.app/Library/Python/${PYTHON_VERSION}/site-packages"
							 "/Applications/qtplatz.app/Library/Python/${PYTHON_VERSION}/site-packages"
							 "/usr/local/lib/python${PYTHON_VERSION}/site-packages"
						   )
		fi
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

echo 'QTPLATZ_DIR: ' ${QTPLATZ_DIR}
echo 'RDKIT_DIR:   ' ${RDKIT_DIR}

export PYTHONPATH=${QTPLATZ_DIR}:${RDKIT_DIR}
echo 'PYTHONPATH:  ' ${PYTHONPATH}

#jupyter notebook
if [ "${command}" == "notebook" ]; then
	python3 -m notebook --no-browser --notebook-dir ${HOME}/src/qtplatz/contrib/jupyter-notebook
else
	python3
fi
