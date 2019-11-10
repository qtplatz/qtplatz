#!/bin/bash

cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/constants.sh
source ${cwd}/prompt.sh
source ${cwd}/nproc.sh

cd $SRC
sudo apt install git
git clone -b master git://git.sv.gnu.org/emacs.git
sudo apt build-dep emacs25

cd $SRC/emacs
./autogen.sh all
