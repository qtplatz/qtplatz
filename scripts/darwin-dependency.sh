#!/bin/bash
brew update
brew list cmake   || brew install cmake
brew list python3 || brew install python3
brew list libxml2 || brew install libxml2
brew list libxslt || brew install libxslt
brew install eigen
brew install rtags

python3 -m pip install numpy
