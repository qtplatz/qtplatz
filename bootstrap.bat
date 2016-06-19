:#!cmd.exe
@echo off

setlocal enabledelayedexpansion

set source_dir=%cd%
set build_root=..
set build_arch=x86_64
set build_type=release
set build_tests=false
set build_clean=false
set address_model=64
set GENERATOR="Visual Studio 14 2015 Win64"

echo "Visual Studio Version: " %VisualStudioVersion%

for %%i in (%*) do (
    if %%i==release (
       set build_type=release
    ) else if %%i==package (
       set build_type=package
    ) else if %%i==tests (
       set build_tests=true
    ) else if %%i==clean (
       set build_clean=true
    ) else if %%i==x86 (
      set build_arch=x86
      set address_model=32
    )
)

if %VisualStudioVersion% EQU 12.0 (
   set tools=vc12
   if %address_model% EQU 32 (
      set GENERATOR="Visual Studio 12 2013"
      set QTDIR=C:\Qt\5.6.0\5.6\msvc2013
   ) else (
      set GENERATORr="Visual Studio 12 2013 Win64"
      set QTDIR=C:\Qt\5.6.0\5.6\msvc2013_64
   )
)

if %VisualStudioVersion% EQU 14.0 (
   set tools=vc14
   if %address_model% EQU 32 (
      set GENERATOR="Visual Studio 14 2015"
      set QTDIR=C:\Qt\5.6\msvc2015
   ) else (
      set GENERATOR="Visual Studio 14 2015 Win64"
      set QTDIR=C:\Qt\5.6\msvc2015_64
   )
)

echo -------------- address_model: !address_model!
echo -------------- arch:          !build_arch!
echo -------------- tools:         !tools!
echo -------------- GENERATOR:     !GENERATOR!

echo "############ bootstrap building qtplatz using "!tools!" #############"

set build_dir=!build_root!\build-!tools!-!build_arch!\qtplatz.!build_type!

if %build_clean%==true (
  echo rmdir !build_dir! /s /q
  rmdir !build_dir! /s /q
  goto end
)

mkdir !build_dir!
cd !build_dir!

if !build_type!==release (
    echo cmake -G !GENERATOR! -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=ON %source_dir%
    cmake -G !GENERATOR! -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=ON %source_dir%
) else if !build_type!==package (
    echo cmake -G !GENERATOR! -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=OFF %source_dir%
    cmake -G !GENERATOR! -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=OFF %source_dir%
    cd contrib\installer\wix
    nmake help
    goto end
)

cd %source_dir%

:end
endlocal
