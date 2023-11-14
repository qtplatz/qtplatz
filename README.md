Visit
	http://www.qtplatz.com

Run macOS binary
================

The executable binary file is on the github.com release.  On the recent version of macOS, it may raise an error to open qtplatz.app ion as “qtplatz.app” is damaged and can’t be opened. You should eject the disk image or “qtplatz”は壊れているため開けません。 ディスクイメージを取り出す必要があります on the Japanese environment. This error could be resolved by issuing the following command from the Terminal.

```bash
xattr -d com.apple.quarantine ~/Desktop/qtplatz.app
```

BUILD PROCEDURE

Prerequisite
===============

1. git
2. cmake 3.18.2
3. c++17 compliant c/c++ compiler (Xcode 12|gcc-6.3|msvc-16)
1. Qt 5.15.2 ([download](https://www.qt.io/download))
2. QWT 6.2
3. Boost-1.73 (also work with 1.69 and later versions)
4. RDKit 2023.03 (optional, but recommended) -- RDKit should be built with a version of boost, which matches the qtplatz build.
5. OpenCV 4.5 (optional)
6. Python3 (optional)
7. netcdf-c (optional; ENABLE_FILTER_BLOSC and ENABLE_FILTER_ZSTD should be disabled or get killed:9 on mac-bundle)

     QtPlatz uses `boost_serialization` for binary data storing on file.  A file serialized by an older version of `boost` can be opened in a newer version of `boost,` but no reverse compatibility.  To check the version of `boost_serialization`, see the `BOOST_ARCHIVE_VERSION` macro value located in the file `boost-source-dir/libs/serialization/src/basic_archive.cpp`.

* Xcode 12 (command-line tools) needs to be installed for macOS.
* Visual Studio 2019 needs to be installed for Windows.
* Qt 5.12 or later version needs to be installed for the target OS.
	* Qt 5.12.2 does not work on Debian based linux -- use Qt 5.12.1 instead.
	* Qt 6 is not supported yet (may it work, though).

Mac macOS 10, 11 and 12 (x86_64)
====================

Prerequisite
--------------
1. Xcode Version 12.3 (12C33)
1. Download the respective qt5 installer from [download](https://www.qt.io/download) page, and follow the instructions.

Install dependencies for macOS (x86_64)
----------------------------

Go to the `<qtplatz-source-dir>/scripts` directory; there are script files for installing dependency software modules.
Assume the qtplatz source is stored under `~/src`

```
cd ~/src/qtplarz/scripts
make dependency
make boost
make qwt
make rdkit
```

* Boost 1.75 (1.79 preferred) is required for the needs of the boost::json library.
* Qwt 6.2.0 is required since QtPlatz Version 5.2.11 and onward.

Build qtplatz
-------------

```
cd ~/src/qtplatz
./bootstrap.sh
cd ~/src/build-Darwin-i386/qtplatz.release # <- build-Linux-x86_64 for Linux
make
```

QtPlatz binary to be built under the `~src/build-Darwin-i386/bin` (`~/src/build-Linux-x86_64/bin`) directory.
Installation for Linux is essentially the same step with macOS.

Mac macOS 10, 11 and 12 (arm64) a.k.a. M1 Mac
====================

First of all, if you got a new M1 Mac computer migrated from your old Intel Mac, it may contain x86_64 binaries under `/usr/local`, etc.
Especially installed libraries for QtPlatz build, such as boost, rdkit, and dependencies, will be detected by the cmake configuration phase but may fail at the link phase because arm64 and x86_64 binary cannot link.  Fortunately, homebrew on M1 Mac installs all files under `/opt/homebrew/` instead of `/usr/local/,` which is less problematic.  However, we also need to use MacPort to install maeparser (rdkit depends on it) and llvm.


Prerequisite
--------------
1. Xcode Version 13.2.1 13C100
1. Download the qt5 source code (either download from [download](https://www.qt.io/download) or git.

Build Qt5 package for arm64 arch
--------------------------------

This step is straightforward but expects one error during the compiling phase.

```
$ cd ~/src
$ git clone git://code.qt.io/qt/qt5.git --branch 5.12
$ cd qt5
$ ./init-repository --module-subset=qtbase
$ git checkout 5.15.2
$ mkdir ~/src/build-Darwin-arm64/qt5-build
$ cd ~/src/build-Darwin-arm64/qt5-build
$ ~/src/qt5/configure -prefix /opt/Qt/5.15.2 QMAKE_APPLE_DEVICE_ARCHS=arm64 -opensource -confirm-license
$ make -j10
```

The above command set may hit a compile error described [here](https://github.com/microsoft/vcpkg/issues/21055)
In such a case, a quick fix is to apply qtplatz/scripts/qt5-5.15.2.patch (or simply add `#include #include <CoreGraphics/CGColorSpace.h>` int `src/plugins/platforms/cocoa/qiosurfacegraphicsbuffer.h` file.  You should be able to build qt5 development files after running 'make' and 'make install' commands.

Install dependencies for macOS (arm64) a.k.a. M1 Mac
----------------------------------------------------

qwt-6.2.0 can be installed as follows;

```bash
/opt/Qt/5.15.2/bin/qmake -r qwt.pro QMAKE_APPLE_DEVICE_ARCHS=arm64
make
sudo make install
```

The rest of the dependent modules can be installed as follows:

```bash
cd ~/src/qtplarz/scripts
make dependency
make boost
port install maeparser
port install llvm
make rdkit
```

Issues on Xcode 15 (version 2379, SDK macOS 14) 
-------------------------------------
Recently released Xcode 15 contains Clang-15 toolchains that break build for at least boost-1_79 through boost-1.82 due to c++-17 removing std::unary_function and std::binary_function, which were marked as deprecated since c++11.  See https://github.com/boostorg/container_hash/issues/22
A quick fix for this is to modify two header files, as listed below.

// boost/functional.hpp line 27
```c++
        namespace detail {
#if defined(_HAS_AUTO_PTR_ETC) && !_HAS_AUTO_PTR_ETC || (BOOST_CXX_VERSION >= 201703)
            // std::unary_function and std::binary_function were both removed
            // in C++17.
```

// boost/container_hash/include/boost/container_hash/hash.hpp
```c++
#if defined(BOOST_NO_CXX98_FUNCTION_BASE) || (BOOST_CXX_VERSION >= 201703)
        template <typename T>
        struct hash_base
        {
            typedef T argument_type;
            typedef std::size_t result_type;
        };
#else
        template <typename T>
        struct hash_base : std::unary_function<T, std::size_t> {};
#endif
```

Note for boost-1_83
-----
boost-1_83 is a recommended boost version with QtPlatz 5.6.1.
An issue previously reported below was resolved.  It happened due to the boost::json change on boost-1.81, where they re-design conversion classes -- it requires application code change, and it has been done as of QtPlatz 5.6.1.
As of 20th Sep. 2023, I have tested with boost-1_83, which seems to have no above issues and is able to be built for boost libraries, maeparser, and RDKit.  However, the compile error was raised in 'boost/json/value.hpp', which uses many of the classes in qtplatz.

Note for qmake with macOS SDK 14
--------
Although already reported on the web pages, execution of qmake command raises the following error so that no generator makefile is generated properly.  The error message isn't clear about the meaning of search paths, whether command search path or library search path, though it seems that it is a dynamic library search path according to old web articles.

````bash
Project ERROR: failed to parse default search paths from compiler output
````
This problem isn't an issue for the qtplatz build; however, it is an issue for building qwt library.

Windows 10 (x64)
===============

Prerequisite for Windows
------------------------
1. Visual Studio 2022
1. [WiX toolset](wixtool.org)
1. [Python](https://www.python.org/downloads/windows/) 3.7 (optional)

Tool for dll dependency diagnostics:
https://github.com/adamrehn/dll-diagnostics

Install dependencies for Windows
--------------------------------

```
cd %USERPROFILE%\src\qtplatz\scripts
.\windows-bootstrap.bat
nmake boost
nmake eigen
nmake maeparser
nmake rdkit
nmake opencv
```

Build qtplatz
--------------
```
cd %USERPROFILE%\src\qtplatz
.\bootstrap.bat package
nmake package
```
