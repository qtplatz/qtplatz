@echo off
set ARCH=
set VC=

echo "##### " ARCH_VC=%ARCH% %VC%
echo "args: " %1 %2 %3

if "%1"=="x64" (
  set ARCH=x64
) else (
  set ARCH=x86
)

if "%2"=="vc12" (
  set VC=vc12
  set QMAKESPEC=win32-msvc2013
) else (
  set VC=vc11
  set QMAKESPEC=win32-msvc2012
)

set uname=%ARCH%_%VC%
echo -------------------------------------
echo ARCH=%ARCH%
echo VC=%VC%
echo arch_tool=%uname%
echo QMAKESPEC=%QMAKESPEC%
echo -------------------------------------

set RDBASE=%USERPROFILE%\src\rdkit

if %uname%==x86_vc11 goto x86_vc11
if %uname%==x86_vc12 goto x86_vc12
if %uname%==x64_vc11 goto x64_vc11
if %uname%==x64_vc12 goto x64_vc12

:x86_vc11
echo ======= setup for x86 32bit memory using VS2012 =================
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86
set ACE_ROOT="%USERPROFILE%\src\vc11\ACE_wrappers"
set QTDIR=C:\Qt\Qt5.1.1\5.1.1\msvc2012
set QWT=C:\Qwt-6.1.1-svn
set BOOST_ROOT=C:\Boost
set BOOST_INCLUDE=%BOOST_ROOT%\include\boost-1_55
set BOOST_LIBRARY=%BOOST_ROOT%
goto all_set

:x86_vc12
echo ======= setup for x86 32bit memory using VS2013 =================
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
set ACE_ROOT="%USERPROFILE%\src\vc11\ACE_wrappers"
set QTDIR=C:\x86\qt5\qtbase
set QWT=C:\x86\vc12\Qwt-6.1.1-svn
set BOOST_ROOT=C:\Boost
set BOOST_INCLUDE=%BOOST_ROOT%\include\boost-1_55
set BOOST_LIBRARY=%BOOST_ROOT%
goto all_set

:x64_vc11
echo ======= setup for x64 64bit memory using VS2012 =================
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" amd64
set ACE_ROOT="%USERPROFILE%\src64\ACE_wrappers"
rem set QTDIR=C:\x64\qt5\qtbase
set QTDIR=C:\x64\Qt5.2.0\5.2.0\msvc2012_64
set QWT=C:\x64\vc11\Qwt-6.1.1-svn
set BOOST_ROOT=C:\Boost
set BOOST_INCLUDE=%BOOST_ROOT%\
set BOOST_LIBRARY=%BOOST_ROOT%\x86_64
goto all_set
:x64_vc12
echo ======= setup for x64 64bit memory using VS2013 =================
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" amd64
set ACE_ROOT="%USERPROFILE%\src64\ACE_wrappers"
set QTDIR=C:\x64\qt5\qtbase
set QWT=C:\x64\vc12\Qwt-6.1.1-svn
set BOOST_ROOT=C:\Boost
set BOOST_INCLUDE=%BOOST_ROOT%\include\boost-1_55
set BOOST_LIBRARY=%BOOST_ROOT%\x86_64
goto all_set

:all_set

set PATH=%ACE_ROOT%\lib;%ACE_ROOT%\bin;%QTDIR%\BIN;%PATH%

echo -- QTDIR set to %QTDIR%
echo -- QMAKESPEC set to %QMAKESPEC%
echo -- ACE_ROOT set to %ACE_ROOT%
echo -- QWT set to %QWT%
echo -- BOOST_ROOT set to %BOOST_ROOT%
echo -- all set.