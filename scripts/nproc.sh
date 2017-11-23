#!/bin/bash

function __nproc() {
    local __resultvar=$1
    local __arch=`uname`
    
    case "${__arch}" in
	Linux*)
	    eval $__resultvar=$(nproc --all)
	    ;;
	Darwin*)
	    eval $__resultvar=$(sysctl -n hw.ncpu)
	    ;;
    esac
}

