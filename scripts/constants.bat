::
@echo off
set BOOST_VERSION=1_67_0
set QMAKE=C:\Qt\5.10.1\msvc2017_64\bin\qmake.exe

pushd %CWD%\..\..
set SOURCE_ROOT=%CD%
popd
set BUILD_ROOT=%SOURCE_ROOT%\build-vc%VisualStudioVersion%-x86_64

if %VisualStudioVersion% EQU 14.0 ( set GENERATOR="Visual Studio 14 2015 Win64" )
if %VisualStudioVersion% EQU 15.0 ( set GENERATOR="Visual Studio 15 2017 Win64" )

set %~1=%BOOST_VERSION%
set %~2=%QMAKE%
set %~3=%SOURCE_ROOT%
set %~4=%BUILD_ROOT%
set %~5=%GENERATOR%
