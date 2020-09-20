Visit
	http://www.qtplatz.com

BUILD PROCEDURE

Prerequisite
===============

1. git
2. cmake 3.18
3. c++17 complient c/c++ compiler


Dependent libraries
=====================

1. Qt 5.15.1 (also work with 5.12 and later versions) ([download](https://www.qt.io/download))
2. QWT 6.1
3. Boost-1.73 (also work with 1.69 and later versions)
4. RDKit 2020.09 (optional, but recommended)
5. OpenCV 3.1 (optional)
6. Python3 (optional)

Linux platform also require following modules:
libxml2-dev, libxslt1-dev, mesa-common-dev, bc, libncurses5-dev libgstreamer-plugins-base0.10

QtPlatz uses `boost_serialization` for binary data storing on file.  The file stored by the older version of `boost` can be opened in a newer version of `boost,` but no reverse compatibility.  To check the version of `boost_serialization`, see the `BOOST_ARCHIVE_VERSION` macro value located in the file `boost-source-dir/libs/serialization/src/basic_archive.cpp`.


Install dependencies
====================

Download respective qt5 installer from [download](https://www.qt.io/download) page, and follow the instraction.

------------------------------------
Dependency installation from source
------------------------------------

Go to `<qtplatz-source-dir>/scripts` directory, there are script files for install dependency software modules.

