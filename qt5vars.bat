@echo off
set ARCH=
set VC=

echo "##### " ARCH_VC=%ARCH% %VC%
echo "args: " %1 %2 %3

if "%1"=="x86" (
  set ARCH=x86
) else (
  set ARCH=x64
)

if "%2"=="vc11" (
  set VC=vc11
  set QMAKESPEC=win32-msvc2012
) else (
  set VC=vc12
  set QMAKESPEC=win32-msvc2013
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

:x64_vc11
echo ======= setup for x64 64bit memory using VS2012 =================
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" amd64
set ACE_ROOT=%USERPROFILE%\src\ACE_wrappers\build\x64
set TAO_ROOT=%ACE_ROOT%\TAO
set QTDIR=C:\Qt\Qt5.2.1\5.2.1\msvc2012_64_opengl
set QWT=C:\x64\Qwt-6.1.1-svn
set BOOST_ROOT=C:\Boost
set BOOST_INCLUDE=%BOOST_ROOT%\include\boost-1_55
set BOOST_LIBRARY=%BOOST_ROOT%\x86_64\lib
goto all_set

:x86_vc11
echo ======= setup for x86 32bit memory using VS2012 =================
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86
set ACE_ROOT=%USERPROFILE%\src\ACE_wrappers\build\x86
set TAO_ROOT=%ACE_ROOT%\TAO
set QTDIR=C:\x86\Qt5.2.1\5.2.1\msvc2012
set QWT=C:\x86\Qwt-6.1.1-svn
set BOOST_ROOT=C:\Boost
set BOOST_INCLUDE=%BOOST_ROOT%\include\boost-1_55
set BOOST_LIBRARY=%BOOST_ROOT%\x86\lib
goto all_set

:x64_vc12
echo ======= setup for x64 64bit memory using VS2013 =================
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" amd64
set ACE_ROOT=%USERPROFILE%\src\ACE_wrappers\build\x64
set TAO_ROOT=%ACE_ROOT%\TAO
set QTDIR=C:\Qt\Qt5.3.0\5.3\msvc2013_64_opengl
set QWT=C:\Qwt-6.1.1-svn
set BOOST_ROOT=C:\Boost
set BOOST_INCLUDE=%BOOST_ROOT%\include\boost-1_55
set BOOST_LIBRARY=%BOOST_ROOT%\x86_64\lib
goto all_set

:x86_vc12
echo ======= setup for x86 32bit memory using VS2013 =================
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86
set ACE_ROOT=%USERPROFILE%\src\ACE_wrappers\build\x86
set TAO_ROOT=%ACE_ROOT%\TAO
set QTDIR=C:\x86\Qt\Qt5.3.0\5.3\msvc2013_64_opengl
set QWT=C:\x86\vc12\Qwt-6.1.1-svn
set BOOST_ROOT=C:\Boost
set BOOST_INCLUDE=%BOOST_ROOT%\include\boost-1_55
set BOOST_LIBRARY=%BOOST_ROOT%\lib
goto all_set

:all_set

set PATH=%ACE_ROOT%\lib;%ACE_ROOT%\bin;%QTDIR%\BIN;%PATH%

echo -- QTDIR set to %QTDIR%
echo -- QMAKESPEC set to %QMAKESPEC%
echo -- ACE_ROOT set to %ACE_ROOT%
echo -- QWT set to %QWT%
echo -- BOOST_ROOT set to %BOOST_ROOT%
echo -- all set.