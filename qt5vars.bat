@echo off

set QTVER=5.2.0
set VC=2012

if "%2"=="vc12" set VC=2013

if "%1"=="x64" goto ARCH_x86_64

:ARCH_x86_32
set ARCH=x32
set ACE_ROOT="%USERPROFILE%\src\vc11\ACE_wrappers"
set QTDIR=C:\Qt\Qt%QTVER%\%QTVER%\msvc2012
set QWT=C:\Qwt-6.1.1-svn
set BOOST_ROOT=C:\Boost
goto ARCH_DONE

:ARCH_x86_64
set ARCH=x64
set ACE_ROOT="%USERPROFILE%\src64\ACE_wrappers"
set QTDIR=C:\x64\qt5\qtbase
set QWT=C:\x64\Qwt-6.1.1-svn
set BOOST_ROOT=C:\x64\Boost

:ARCH_DONE

set PATH=%ACE_ROOT%\lib;%ACE_ROOT%\bin;%QTDIR%\BIN;%PATH%

if "%VC%"=="2013" goto VC2013
:VC2012

set QMAKESPEC=win32-msvc2012
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" %ARCH%
goto VCVARS_DONE

:VC2013
set QMAKESPEC=win32-msvc2013
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" %ARCH%

:VCVARS_DONE

echo -- QTDIR set to %QTDIR%
echo -- QMAKESPEC set to %QMAKESPEC%
echo -- ACE_ROOT set to %ACE_ROOT%
echo -- QWT set to %QWT%
echo -- BOOST_ROOT set to %BOOST_ROOT%

if not "%1"=="vsstart" goto ENDVSSTART
devenv /useenv
:ENDVSSTART

