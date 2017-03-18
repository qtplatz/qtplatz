#!/bin/sh

if [ ! -d ~/src/rdkit ]; then
    git clone https://github.com/rdkit/rdkit ~/src/rdkit
fi

if [ -z $BOOST_ROOT ]; then
    if [ -d /usr/local/boost-1_63 ]; then
	BOOST_ROOT=/usr/local/boost-1_63
    elif [ -d /usr/local/boost-1_62 ]; then
	BOOST_ROOT=/usr/local/boost-1_62
    fi
fi

cd ~/src/rdkit
export RDBASE=`pwd`

echo "RDKit install on $RDBASE"

mkdir build;
cd build
echo "Current working directory: $(pwd)"

if [ -z BOOST_ROOT ]; then
    cmake -DRDK_BUILD_INCHI_SUPPORT=ON -DRDK_BUILD_PYTHON_WRAPPERS=OFF ..
else
    cmake -DBOOST_ROOT=$BOOST_ROOT -DRDK_BUILD_INCHI_SUPPORT=ON -DRDK_BUILD_PYTHON_WRAPPERS=OFF ..
fi
make -j8
make test
make install
