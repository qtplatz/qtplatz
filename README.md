Visit
	http://www.qtplatz.com

BUILD PROCEDURE

Prerequisite
===============

1. git
2. cmake 3.18
3. c++17 complient c/c++ compiler
1. Qt 5.15.1 (also work with 5.12 and later versions) ([download](https://www.qt.io/download))
2. QWT 6.1
3. Boost-1.73 (also work with 1.69 and later versions)
4. RDKit 2020.09 (optional, but recommended)
5. OpenCV 3.1 (optional)
6. Python3 (optional)

QtPlatz uses `boost_serialization` for binary data storing on file.  The file stored by the older version of `boost` can be opened in a newer version of `boost,` but no reverse compatibility.  To check the version of `boost_serialization`, see the `BOOST_ARCHIVE_VERSION` macro value located in the file `boost-source-dir/libs/serialization/src/basic_archive.cpp`.

* Xcode 12 (command-line tools) needs to be installed for macOS.
* Visual Studio 2019 needs to be installed for Windows.
* Qt 5.12 or later version needs to be installed for the target OS.

Install on Mac
====================

Download the respective qt5 installer from [download](https://www.qt.io/download) page, and follow the instructions.

Go to `<qtplatz-source-dir>/scripts` directory; there are script files for install dependency software modules.
Assume qtplatz source is stored under `~/src`


```
cd ~/src/qtplarz/scripts
make dependency
make boost
make qwt
make rdkit
```

Install qtplatz
----------------

```
cd ~/src/qtplatz
./bootstrap.sh
cd ~/src/build-Darwin-i386/qtplatz.release # <- build-Linux-x86_64 for Linux
make
```

QtPlatz binary to be built under `~src/build-Darwin-i386/bin` (`~/src/build-Linux-x86_64/bin`) directory.

Install for Linux is essentially the same step with macOS.


Windows
===========

```
cd %USERPROFILE%\src\qtplatz\scripts
.\bootstrap-windows.bat
```

```
cd %USERPROFILE%\src\qtplatz
./bootstrap.bat package
build-package.bat
```
