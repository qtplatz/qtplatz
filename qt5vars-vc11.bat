@echo off

set VER=5.1.1

set ACE_ROOT="%USERPROFILE%\src\vc11\ACE_wrappers"
set QTDIR=C:\Qt\Qt%VER%\%VER%\msvc2012
set QMAKESPEC=win32-msvc2012
set PATH=%ACE_ROOT%\lib;%ACE_ROOT%\bin;%QTDIR%\BIN;%PATH%

echo -- QTDIR set to %QTDIR%
echo -- QMAKESPEC set to %QMAKESPEC%
echo -- ACE_ROOT set to %ACE_ROOT%

call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86

if not "%1"=="vsstart" goto ENDVSSTART
devenv /useenv
:ENDVSSTART

