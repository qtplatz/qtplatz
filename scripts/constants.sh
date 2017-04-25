#!/bin/bash

if [ -z $SRC ]; then
    SRC=~/src
fi

if [ -z $DOWNLOADS ]; then
    DOWNLOADS=~/Downloads
fi

function find_QMAKE() {
    local __result=$1
    local dirs=( $QTDIR "/opt/Qt/5.8/gcc_64" "/opt/Qt/5.7/gcc_64" "/opt/Qt/5.6/gcc_64" )
    
    if [ -z $QTDIR ]; then
	if type -P qmake &> /dev/null && qmake --version &> /dev/null; then
	    QMAKE=qmake
	    return 0
	fi
    fi
    
    for dir in "${dirs[@]}"; do
	if $dir/bin/qmake --version &> /dev/null ; then
	    eval $__result="'$dir/bin/qmake'"
	    return 0
	fi
    done

    return 1
}
