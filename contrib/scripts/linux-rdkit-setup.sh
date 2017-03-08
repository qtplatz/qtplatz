#!/bin/sh

if [ ! -d ~/src/rdkit ]; then
    git clone https://github.com/rdkit/rdkit ~/src/rdkit
fi

cd ~/src/rdkit
export RDBASE=`pwd`

echo "RDKit install on $RDBASE"

mkdir build;
cd build
echo "Current working directory: $(pwd)"
cmake -DBOOST_ROOT=/usr/local/boost-1_63 -DRDK_BUILD_INCHI_SUPPORT=ON -DRDK_BUILD_PYTHON_WRAPPERS=OFF ..
make -j8
make test
make install
