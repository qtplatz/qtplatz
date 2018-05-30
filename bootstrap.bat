::#!cmd.exe
@echo off

set "source_dir=%~dp0"
pushd "%~dp0.."
set build_root=%CD%
popd

if not defined QTDIR (
   set QMAKE=
   call "%source_dir%scripts\find_qmake.bat" QMAKE
   if not defined QMAKE ( echo "************ No QMAKE found." & goto end )
   echo "********** QMAKE=%QMAKE%"
)

set build_arch=x86_64
set build_target=release
set build_package=
set build_clean=
set tools=%VisualStudioVersion%
set debug_symbol="ON"

setlocal enabledelayedexpansion

if %VisualStudioVersion% EQU 15.0 (
   set GENERATOR="Visual Studio 15 2017 Win64"
) else (
  echo "Not supported compiler version"
  goto end
)

for %%i in (%*) do (
    if %%i==package (
       set build_package=y
       set build_clean=y
       set debug_symbol="OFF"
    ) else if %%i==clean (
       set build_clean=y
    )      
)

set build_dir=%build_root%\build-%build_arch%\qtplatz.release
pushd %build_dir%
build_dir=%CD%
popd

echo -- arch:      %build_arch%
echo -- tools:     %tools%
echo -- GENERATOR: %GENERATOR%
echo -- BUILD DIR: %build_dir%
echo -- PACKAGE  : %build_package%
echo -- CLEAN    : %build_clean%
echo -- QMAKE    : %QMAKE%

set /p Yes=Proceed (y/n)?
if /i Yes neq "y" goto end

if defined build_clean (
   echo rmdir %build_dir% /s /q
   rmdir %build_dir% /s /q
   if not defined build_package ( goto end )
)

if not exist %build_dir% ( mkdir %build_dir% )
cd %build_dir%

echo cmake -DQTDIR=%QTDIR% -G %GENERATOR% -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=%debug_symbol% %source_dir%

cmake -DQTDIR=%QTDIR% -G %GENERATOR% -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=%debug_symbol% %source_dir%

echo "============ endlocal =============="

endlocal && set build_dir=%build_dir%

pushd %build_dir%
echo --
echo -- You are in the %build_dir%
echo -- 'popd' command take you back to source directory.
echo --

:end
