@echo off
set BOOST_VERSION=
set QMAKE=
set BUILD_ROOT=
set SOURCE_ROOT=
set GENERATOR=
set CWD="%~dp0"
call %CWD%\constants.bat BOOST_VERSION QMAKE SOURCE_ROOT BUILD_ROOT GENERATOR

for /F "tokens=1,2,3 delims=_" %%a in ("%BOOST_VERSION%") do (
    set BOOST_VERSION_Major=%%a
    set BOOST_VERSION_Minor=%%b
    set BOOST_VERSION_Revision=%%c
)

set BOOST_ROOT=C:\Boost\include\boost-%BOOST_VERSION_Major%_%BOOST_VERSION_Minor%_%BOOST_VERSION_Revision%
set BOOST_LIBRARY_DIR=C:\Boost\lib
set BOOST_BUILD_DIR=%SOURCE_ROOT%\boost_%BOOST_VERSION%
set BZIP2_SOURCE_DIR=%SOURCE_ROOT%\bzip2-1.0.6

echo "Installing BOOST: BOOST_VERSION=%BOOST_VERSION%"
echo "Install to      : BOOST_ROOT=%BOOST_ROOT%"
echo "Buld in         : BOOST_BUILD_DIR=%BOOST_BUILD_DIR%"
echo "Additional      : BZIP2_SOURCE_DIR=%BZIP2_SOURCE_DIR%"

if %VisualStudioVersion% EQU 14.0 ( set msvc=msvc-14.0 )
if %VisualStudioVersion% EQU 15.0 ( set msvc=msvc-14.1 )

:Expecting git-bash installed, and enable tools from windows command prompt

if NOT EXIST %BZIP2_SOURCE_DIR% (
   echo "%BZIP2_SOURCE_DIR% does not exist."
   set tarball=bzip2-1.0.6.tar.gz
   if NOT EXIST %SOURCE_ROOT%/%tarball% (
      echo "downloading http://www.bzip.org/1.0.6/%tarball%"
      curl -L -o %SOURCE_ROOT%/%tarball% http://www.bzip.org/1.0.6/%tarball%
      pushd %SOURCE_ROOT%
      tar xvf %tarball%
      popd
   )
)

if NOT EXIST %BOOST_BUILD_DIR% (
   set tarball=boost_%BOOST_VERSION%.tar.bz2
   set VERSION=%BOOST_VERSION_Major%.%BOOST_VERSION_Minor%.%BOOST_VERSION_Revision%
   if NOT EXIST %SOURCE_ROOT%\%tarball% (
      echo "downloading https://sourceforge.net/projects/boost/files/boost/%VERSION%/%tarball%/download" "-->" "%SOURCE_ROOT%"
      pushd %SOURCE_ROOT%
      curl -L -o %tarball% https://sourceforge.net/projects/boost/files/boost/%VERSION%/%tarball%
      popd
   )
   pushd %SOURCE_ROOT%
   tar xvf %tarball%
   popd
)

if NOT EXIST %BOOST_BUILD_DIR% (
   echo "DIRECTORY %BOOST_BUILD_DIR% does not exist"
   goto end
)

pushd %BOOST_BUILD_DIR%
call bootstrap.bat

if %BOOST_VERSION_Minor% gtr "65" (
   goto simple
)

b2 -j%nproc% toolset=%msvc% architecture=x86 address-model=64 -s BZIP2_SOURCE=%bzip2_dir% threading=multi runtime-link=shared --build-type=minimal link=static --stagedir=stage/x86_64 stage
b2 -j%nproc% toolset=%msvc% architecture=x86 address-model=64 -s BZIP2_SOURCE=%bzip2_dir% threading=multi runtime-link=shared --build-type=minimal link=shared --stagedir=stage/x86_64 stage install

goto end

:simple
b2 -j%NUMBER_OF_PROCESSORS% address-model=64 -s BZIP2_SOURCE=%bzip2_dir% link=static --stagedir=stage/x64-static stage install
b2 -j%NUMBER_OF_PROCESSORS% address-model=64 -s BZIP2_SOURCE=%bzip2_dir% link=shared --stagedir=stage/x64-shared stage install

:end

cd %CWD%
