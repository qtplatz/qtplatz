#!/bin/sh
BIN_FILE=$1
echo $BIN_FILE

for P in `otool -L $1 | awk '{print $1}'` 
do 
    echo $P
    if [[ "$P" == *//* ]] 
    then 
        PSLASH=$(echo $P | sed 's,//,/,g')
        install_name_tool -change $P $PSLASH $BIN_FILE
    fi 
done 