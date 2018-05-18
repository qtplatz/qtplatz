:@echo off

if %VisualStudioVersion% EQU 14.0 (
   set GENERATOR="Visual Studio 14 2015 Win64"
   set build_dir=%src_dir%\build-vc14-x86_64
   set qmake=c:\Qt\5.9.3\msvc2015_64\bin\qmake.exe
)
if %VisualStudioVersion% EQU 15.0 (
   set GENERATOR="Visual Studio 15 2017 Win64"
   set build_dir=%src_dir%\build-vc15-x86_64
   set qmake=c:\Qt\5.9.3\msvc2017_64\bin\qmake.exe
)

set CWD=%cd%
set nproc=%NUMBER_OF_PROCESSORS%
set src_dir=%USERPROFILE%\source
set qwt_dir=%src_dir%\qwt-6.1

if not exist %qwt_dir% (
   cd %src_dir%
   svn checkout svn://svn.code.sf.net/p/qwt/code/branches/qwt-6.1
)

cd %qwt_dir%
if not exist "qwtconfig.pri.orig" (
   ren qwtconfig.pri qwtconfig.pri.orig
)

sed 's/\([ \t]*QWT_CONFIG.*QwtDesigner\)/#\1/; s/\([ \t]*QWT_CONFIG.*QwtDll\)/#\1/' qwtconfig.pri.orig > qwtconfig.pri

%qmake% qwt.pro
nmake
nmake install

cd %CWD%
