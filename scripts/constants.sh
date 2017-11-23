#!/bin/bash

if [ -z $SRC ]; then
    SRC=~/src
fi

if [ -z $DOWNLOADS ]; then
    DOWNLOADS=~/Downloads
fi

. ./find_qmake.sh
