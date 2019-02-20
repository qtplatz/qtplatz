#!/bin/bash

function find_qmake() {
    local __arch=`uname`
    local __result=$1

    local hints=( "/Qt/5.12.1" "/Qt/5.12.0" \
					  "/Qt/5.11.2" "/Qt/5.11.1" "/Qt/5.11.0" \
					  "/Qt/5.10.1" \
					  "/Qt/5.9.3" "/Qt/5.9.2" "/Qt/5.9.1" "/Qt/5.9" \
					  "/Qt/5.8" \
					  "/Qt/5.7" )

    case "${__arch}" in
	Linux*)
	    local __dirs=()
	    for hint in "${hints[@]}"; do
		__dirs+=("/opt$hint/gcc_64")
	    done
	    ;;
	Darwin*)
	    local home=~
	    local __dirs=()
	    for hint in "${hints[@]}"; do
		__dirs+=("$home$hint/clang_64")
	    done	    
	    ;;
	*)
	    echo "######## unknown arch: " $__arch
	    ;;
    esac

    for dir in "${__dirs[@]}"; do
	if [ -f $dir/bin/qmake ]; then
	    if $dir/bin/qmake --version &> /dev/null ; then
		eval $__result="'$dir/bin/qmake'"
		return 0; #true
	    fi
	fi
    done
    return 1; #false
}

