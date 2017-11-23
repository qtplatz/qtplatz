#!/bin/bash

function find_qmake() {
    local __arch=`uname`
    local __result=$1

    case "${__arch}" in
	Linux*)
	    local dirs=( "/opt/Qt/5.9.3/gcc_64" "/opt/Qt/5.9.2/gcc_64" "/opt/Qt/5.9.1/gcc_64" "/opt/Qt/5.9/gcc_64" \
					  "/opt/Qt/5.8/gcc_64" \
					  "/opt/Qt/5.7/gcc_64" )
	    ;;
	Darwin*)
	    local home=~
	    local dirs=( "${home}/Qt/5.9.3/clang_64" "${home}/Qt/5.9.2/clang_64" "${home}/Qt/5.9.1/clang_64" "${home}/Qt/5.9/clang_64" \
					       "${home}/Qt/5.8/clang_64" \
					       "${home}/Qt/5.7/clang_64" )
	    ;;
	*)
	    echo "######## unknown arch: " $__arch
    esac

    for dir in "${dirs[@]}"; do
	if $dir/bin/qmake --version &> /dev/null ; then
	    eval $__result="'$dir/bin/qmake'"
	    return 0; #true
	fi
    done
    return 1; #false
}

