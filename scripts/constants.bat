::
@echo off

set "cwd=%~dp0"

set BOOST_VERSION=1_85_0
::set BOOST_VERSION=1_83_0
::set BOOST_VERSION=1_79_0

call %cwd%\find_qmake QMAKE

pushd %~dp0..\..
set SOURCE_ROOT=%CD%
popd
set BUILD_ROOT=%SOURCE_ROOT%\build-x86_64

set GENERATOR="NMake Makefiles"

set "a1=%~1"
set "a2=%~2"
set "a3=%~3"
set "a4=%~4"
set "a5=%~5"

set %a1%=%BOOST_VERSION%
set %a2%=%QMAKE%
set %a3%=%SOURCE_ROOT%
set %a4%=%BUILD_ROOT%
set %a5%=%GENERATOR%
