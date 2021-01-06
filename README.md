Visit
	http://www.qtplatz.com

BUILD PROCEDURE

Prerequisite
===============

1. git
2. cmake 3.18.2
3. c++17 complient c/c++ compiler (Xcode 12|gcc-6.3|msvc-16)
1. Qt 5.15.1 (also work with 5.12 and later versions) ([download](https://www.qt.io/download))
2. QWT 6.1
3. Boost-1.73 (also work with 1.69 and later versions)
4. RDKit 2020.09 (optional, but recommended) -- RDKit should be built with a version of boost, which match to qtplatz build.
5. OpenCV 4.5 (optional)
6. Python3 (optional)

     QtPlatz uses `boost_serialization` for binary data storing on file.  A file serialized by an older version of `boost` can be opened in a newer version of `boost,` but no reverse compatibility.  To check the version of `boost_serialization`, see the `BOOST_ARCHIVE_VERSION` macro value located in the file `boost-source-dir/libs/serialization/src/basic_archive.cpp`.

* Xcode 12 (command-line tools) needs to be installed for macOS.
* Visual Studio 2019 needs to be installed for Windows.
* Qt 5.12 or later version needs to be installed for the target OS.
	* Qt 5.12.2 does not work on Debian based linux -- use Qt 5.12.1 instead.
	* Qt 6 is not supported yet.

Mac macOS 10 and 11
====================

Prerequisite
--------------
1. Xcode Version 12.3 (12C33)
1. Download the respective qt5 installer from [download](https://www.qt.io/download) page, and follow the instructions.

Install dependencies for Mac
----------------------------

Go to `<qtplatz-source-dir>/scripts` directory; there are script files for install dependency software modules.
Assume qtplatz source is stored under `~/src`

```
cd ~/src/qtplarz/scripts
make dependency
make boost
make qwt
make rdkit
```

Build qtplatz
-------------

```
cd ~/src/qtplatz
./bootstrap.sh
cd ~/src/build-Darwin-i386/qtplatz.release # <- build-Linux-x86_64 for Linux
make
```

QtPlatz binary to be built under `~src/build-Darwin-i386/bin` (`~/src/build-Linux-x86_64/bin`) directory.
Install for Linux is essentially the same step with macOS.


Windows 10 (x64)
===============

Prerequisite for Windows
------------------------
1. Visual Studio 2019
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
