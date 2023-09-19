#!/bin/bash

function find_qmake() {
    local __uname=`uname`
    local __result=$1

    case "${__uname}" in
		Linux*)
			local hints=( "/Qt/5.15.2" "/Qt/5.15.1" )
			;;
		*)
			local hints=( "/Qt5/5.15.2"	"/Qt/5.15.2" "/Qt/5.15.1" )
			;;
	esac

    case "${__uname}" in
	Linux*)
	    local __dirs=()
	    for hint in "${hints[@]}"; do
			__dirs+=("/opt$hint/gcc_64")
	    done
	    __dirs+=("/usr")
	    ;;
	Darwin*)
	    local home=~
	    local __dirs=()
	    for hint in "${hints[@]}"; do
			__dirs+=("$home$hint/clang_64")
			__dirs+=("/opt$hint/clang_64")
			__dirs+=("/opt$hint")
	    done
	    ;;
	MINGW64_NT*)
		for hint in "${hints[@]}"; do
			__dirs+=("/c/$hint/mingw81_64")
		done
		;;
	*)
	    echo "######## unknown arch: " $__uname
	    ;;
    esac

    for dir in "${__dirs[@]}"; do
		#echo "-----------" $dir
		if [ -f $dir/bin/qmake ]; then
			if $dir/bin/qmake --version &> /dev/null ; then
				eval $__result="'$dir/bin/qmake'"
				return 0; #true
			fi
		fi
    done
    return 1; #false
}
