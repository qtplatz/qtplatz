#!/bin/sh
echo "#define VERSION \"`git describe`\"" > $1
