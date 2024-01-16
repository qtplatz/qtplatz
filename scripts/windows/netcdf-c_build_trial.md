zlib
====
download zlib-1.3 source
cd zlib-1.3
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX="/opt/zlib-1.3" ..
cmake --build . --config Release --target INSTALL

hdf5
====
download hdf5-1.14.3.zip and unzip it under ~/src
cd hdf5-1.14.3
mkdir build
cd build
cmake -DZLIB_ROOT="C:\opt\zlib-1.3" ..

(Although, error message show as '(missing ZLIB_DIR)', which is wrong information.  Since CMakeFilters.txt uses find_package( ZLIB) that fall in to CMake's native FindZLIB.cmake. FindZLIB.cmake cleary not that user should set ZLIB_ROOT variable.)

cmake --build . --config Release --target PACKAGE
>>> msi package will be created.


netcdf-c require m4.exe and curl package;
m4.exe download binary from https://gnuwin32.sourceforge.net/packages/m4.htm

curl: wget source from https://curl.se/download.html
cmake -DCMAKE_INSTALL_PREFIX="c:/opt/curl-8.5.0" -DZLIB_ROOT="c:/opt/zlib-1.3" -DCURL_DIR="C:/opt/curl-8.5.0/lib/cmake" ..
cmake --build . --config Release --target INSTALL

netcdf-c
========

cmake -DZLIB_ROOT="c:/opt/zlib-1.3" -DCMAKE_PREFIX_PATH="c:/opt/curl-8.5.0/lib/cmake;C:\Program Files\HDF_Group\HDF5\1.14.3\cmake" -D"ENABLE_NETCDF_4=ON" -D"ENABLE_DAP=OFF" -D"BUILD_UTILITIES=ON" -D"BUILD_SHARED_LIBS=OFF" ..

The build failed on netcdf3.vcxproj step...
