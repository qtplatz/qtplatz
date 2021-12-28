#!/bin/bash
cwd="$(cd "$(dirname "$0")" && pwd)"
source ${cwd}/config.sh
source ./prompt.sh
arch=`arch`

target=rtags
config=release

if type rdm > /dev/null; then
    version=$(rdm --version)
    echo "=========="
    echo "rdm $version already installed"
    prompt
fi

echo "=========="
echo "building rtags on the directory '$SRC'"
echo "Try: 'apt-get install libclang-3.8-dev' build failed with <clang-c/Index.h> matter"

if [ ! -d $SRC ]; then
    echo "$SRC not exist.  Create?"
    prompt
    mkdir -p $SRC
fi

if [ ! -d $SRC/rtags ]; then
    ( cd $SRC;
      git clone --recursive https://github.com/Andersbakken/rtags.git )
fi

source_dir=$SRC/rtags
build_dir=$SRC/build-$arch/$target.$config

mkdir -p $build_dir
cd $build_dir

#emacs-27 may fail for bitcompile due to no cl package
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DRTAGS_NO_ELISP_BYTECOMPILE=1 $source_dir

if make -j4; then
    echo "==================================================="
    echo "'sudo make install' at `pwd`"
    echo "==================================================="
    prompt
    sudo make install
fi

echo "==================================================="
echo "start rtags daemon by typing 'rdm &'"
echo "Index project by run 'rc -J .' at project dirctory"
echo "==================================================="
