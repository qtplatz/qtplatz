@echo off

set BOOST_VERSION=
set QMAKE=
set BUILD_ROOT=
set SOURCE_ROOT=
set GENERATOR=
set CWD="%~dp0"

call %CWD%\constants.bat BOOST_VERSION QMAKE SOURCE_ROOT BUILD_ROOT GENERATOR

set BUILD_DIR=%SOURCE_ROOT%\windows.build
set SOURCE_DIR=%CWD%\windows
echo "##############################################"
echo "CWD=%CWD%"
echo "BUILD_DIR=%BUILD_DIR%"
echo "##############################################"

if not exist %BUILD_DIR% (
   echo mkdir "%BUILD_DIR%"
   mkdir "%BUILD_DIR%"
)

cmake -G "NMake Makefiles" %SOURCE_DIR%

pushd %BUILD_DIR%
nmake help
