mkdir ..\..\rdkit\build
cd ..\..\rdkit\build
cmake -DBOOST_LIBRARYDIR=C:/Boost/lib -DBOOST_ROOT=C:/Boost/include/boost-1_65_1 -DBoost_USE_STATIC_LIBS=ON -DRDK_BUILD_INCHI_SUPPORT=ON -DRDK_BUILD_PYTHON_WRAPPERS=OFF -DCMAKE_DEBUG_POSTFIX="d" -G "Visual Studio 15 2017 Win64" ..
