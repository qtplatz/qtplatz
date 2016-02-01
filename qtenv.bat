@echo off
if %VisualStudioVersion% EQU 14.0 (
   set QTDIR=C:\Qt\Qt5.6.0\5.6\msvc2015_64
   echo QTDIR=%QTDIR%
   )

if %VisualStudioVersion% EQU 12.0 (
   set QTDIR=C:\Qt\5.5\msvc2013_64
   echo QTDIR=%QTDIR%
   }

if exist %QTDIR%\bin set PATH=%QTDIR%\bin;"%PATH%"

