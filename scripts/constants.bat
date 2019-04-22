::
@echo off

set BOOST_VERSION=1_67_0
call find_qmake

pushd %~dp0..\..
set SOURCE_ROOT=%CD%
popd
set BUILD_ROOT=%SOURCE_ROOT%\build-x86_64

set GENERATOR="NMake Makefiles"

set %~1=%BOOST_VERSION%
set %~2=%QMAKE%
set %~3=%SOURCE_ROOT%
set %~4=%BUILD_ROOT%
set %~5=%GENERATOR%
