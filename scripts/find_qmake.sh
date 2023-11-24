#!/bin/bash

function find_qmake() {
    local __uname=`uname`
    local __result=$1
	local __dirs=()

    case "${__uname}" in
		Linux*)
			local hints=( "6.5.3" "5.15.2" )
			;;
		*)
			local hints=( "/Qt/6.5.3/macos" "/Qt5/5.15.2" "/Qt/5.15.2/clang_64" )
			;;
	esac

    case "${__uname}" in
	Linux*)
	    for hint in "${hints[@]}"; do
			__dirs+=("/opt/Qt/$hint/gcc_64")
	    done
	    __dirs+=("/usr")
	    ;;
	Darwin*)
		local __pfxs=(~ "/opt")
		for pfx in "${__pfxs[@]}"; do
			for hint in "${hints[@]}"; do
				__dirs+=("${pfx}${hint}")
			done
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
			#echo "----------- found file: $dir/bin/qmake"
			if $dir/bin/qmake --version &> /dev/null ; then
				eval $__result="'$dir/bin/qmake'"
				return 0; #true
			fi
		fi
    done
	echo "----------- qmake cannot be found ----------"
    return 1; #false
}
