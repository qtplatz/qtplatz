:#!cmd.exe
@echo off

set source_dir="%~dp0"
set build_root="%~dp0.."
set build_arch=x86_64
set build_target=release
set build_tests=false
set build_clean=false
set query_build_dir=false
set exec_build=false
set tools=%VisualStudioVersion%

setlocal enabledelayedexpansion
call %~dp0%\constants.bat

if %VisualStudioVersion% EQU 15.0 (
   set GENERATOR="Visual Studio 15 2017 Win64"
) else (
  echo "Not supported compiler version"
  goto end
)

for %%i in (%*) do (
    if %%i==release (
       set build_target=release
    ) else if %%i==package (
       set build_target=package
    ) else if %%i==tests (
       set build_tests=true
    ) else if %%i==clean (
       set build_clean=true
    ) else if %%i==x86 (
      set build_arch=x86
    ) else if "%%i"=="--query-source_dir" (
      @echo %source_dir%
      goto end
    ) else if "%%i"=="--query-build_root" (
      @echo %build_root%
      goto end
    ) else if "%%i"=="--query-build_dir" (
      set query_build_dir=true
    ) else if "%%i"=="--build" (
      set exec_build=true
    ) else if "%%i"=="--package" (
      set exec_build=true
      set build_target=package
    )      
)

set build_dir=!build_root!\build-!tools!-!build_arch!\qtplatz.!build_target!
pushd !build_dir!
build_dir=%CD%
popd

if %query_build_dir%==true (
   @echo %build_dir%
   goto end
)

echo -------------- arch:          !build_arch!
echo -------------- tools:         !tools!
echo -------------- GENERATOR:     !GENERATOR!

echo "############ bootstrap building qtplatz using "!tools!" #############"

if %build_clean%==true (
  echo rmdir !build_dir! /s /q
  rmdir !build_dir! /s /q
  goto end
)

if not exist !build_dir! (
   mkdir !build_dir!
)

cd !build_dir!

if !build_target!==release (
    echo cmake -G !GENERATOR! -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=ON %source_dir%
    cmake -G !GENERATOR! -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=ON %source_dir%
    cd %source_dir%
) else if !build_target!==package (
    echo cmake -G !GENERATOR! -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=OFF %source_dir%
    cmake -G !GENERATOR! -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=OFF %source_dir%
)

if "%exec_build%"=="true" (
   nmake -C !build_dir! !build_target!
)

:end

endlocal && set build_dir=%build_dir%

pushd %build_dir%
echo --
echo -- You are in the %build_dir%
echo -- 'popd' command take you back to source directory.
echo --

