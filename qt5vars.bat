@echo off

set VER=5.2.0
set ARCH=x64
set VC=msvc2013

set ACE_ROOT="%USERPROFILE%\src\vc11\ACE_wrappers"
set QTDIR=C:\Qt\Qt%VER%\%VER%\msvc-2012
set PATH=%ACE_ROOT%\lib;%ACE_ROOT%\bin;%QTDIR%\BIN;%PATH%

echo -- QTDIR set to %QTDIR%
echo -- QMAKESPEC set to %QMAKESPEC%
echo -- ACE_ROOT set to %ACE_ROOT%

if "%VC%"=="msvc2013" goto VC2013
set QMAKESPEC=win32-msvc2012
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" %ARCH%
goto END
:VC2013
set QMAKESPEC=win32-msvc2013
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" %ARCH%
:END

if not "%1"=="vsstart" goto ENDVSSTART
devenv /useenv
:ENDVSSTART

