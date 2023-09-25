
INSTALL mingw cross environment
===========

```bash
sudo apt install binutils-mingw-w64-x86-64
sudo apt install mingw-w64-common
sudo apt install mingw-w64-x86-64-dev
sudo apt install mingw-w64-tools
sudo apt install gcc-mingw-w64-base
sudo apt install gcc-mingw-w64-x86-64
sudo apt install g++-mingw-w64-x86-64
```

```bash
update-alternatives --set x86_64-w64-mingw32-g++ /usr/bin/x86_64-w64-mingw32-g++-posix
update-alternatives --set x86_64-w64-mingw32-gcc /usr/bin/x86_64-w64-mingw32-gcc-posix
```

INSTALL Qt
============

```bash
$ mkdir ~/src/build-build-x86_64-w64-mingw32
$ cd ~/src/build-build-x86_64-w64-mingw32

$ /opt/Qt/5.15.2/Src/configure -opensource -confirm-license -xplatform win32-g++ -device-option CROSS_COMPILE=/usr/bin/x86_64-w64-mingw32- -prefix /opt/Qt/5.15.2/x86_64-w64-mingw32 -nomake examples -opengl desktop -I/usr/x86_64-w64-mingw32/include/ -L/usr/x86_64-w64-mingw32/lib QMAKE_LIBS+="-lws2_32"
make -j8 -k
```

-I/opt/Qt/5.15.2/Src/qtactiveqt/src/tools/idc -I. -I/opt/Qt/5.15.2/Src/qtbase/include -I/opt/Qt/5.15.2/Src/qtbase/include/QtCore
-I/opt/Qt/5.15.2/Src/qtbase/include/QtCore/5.15.2 -I/opt/Qt/5.15.2/Src/qtbase/include/QtCore/5.15.2/QtCore
-I/opt/Qt/5.15.2/Src/qtbase/include/QtXml
-I/opt/Qt/5.15.2/Src/qtbase/include/QtXml/5.15.2
-I/opt/Qt/5.15.2/Src/qtbase/include/QtXml/5.15.2/QtXml
-I/home/toshi/src/build-x86_64-w64-mingw32/qt/qtbase/include
-I/home/toshi/src/build-x86_64-w64-mingw32/qt/qtbase/include/QtCore
-I/home/toshi/src/build-x86_64-w64-mingw32/qt/qtbase/include/QtXml
-I/opt/Qt/5.15.2/Src/qtbase/mkspecs/linux-g++ -o .obj/main.o /opt/Qt/5.15.2/Src/qtactiveqt/src/tools/idc/main.cpp
