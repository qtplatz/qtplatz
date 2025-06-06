::#!cmd.exe
@echo off

set "source_dir=%~dp0"

call "%source_dir%scripts\find_qmake.bat" QMAKE
for /f "tokens=*" %%a in ( '%QMAKE% -query QT_INSTALL_PREFIX' ) do ( set "QTDIR=%%a" )

pushd "%~dp0.."
set build_root=%CD%
popd

if not defined QMAKE (
   echo "----------------- QMAKE NOT DEFINED ----------------"
   echo "## No QMAKE found." & goto end
)

set build_arch=x86_64
set build_target=qtplatz.release
set build_package=
set build_clean=
set toolset=
set tools=%VisualStudioVersion%
set debug_symbol="ON"

setlocal enabledelayedexpansion

if %VisualStudioVersion% EQU 15.0 (
   set GENERATOR="Visual Studio 15 2017 Win64"
) else if %VisualStudioVersion% EQU 16.0 (
   set GENERATOR="Visual Studio 16 2019"
) else if %VisualStudioVersion% EQU 17.0 (
   set GENERATOR="Visual Studio 17 2022"
) else (
  echo "Not supported compiler version"
  goto end
)

for %%i in (%*) do (
    if %%i==package (
       set build_package=yes
       set build_clean=yes
       set debug_symbol="OFF"
    ) else if "%%i"=="clean" (
       set build_clean=yes
    ) else if "%%i"=="ClangCL" (
       set toolset=ClangCL
    ) else if "%%i"=="Ninja" (
       set GENERATOR="Ninja"
    )
)

set "build_dir=%build_root%\build-%build_arch%\%build_target%"

echo "<==============================="
echo -- GENERATOR: %GENERATOR%
echo -- BUILD DIR: %build_dir%
echo -- PACKAGE  : %build_package%
echo -- CLEAN    : %build_clean%
echo -- QMAKE    : %QMAKE%
echo -- QTDIR    : %QTDIR%
echo -- toolset  : %toolset%
echo "<==============================="

::set /p Yes=Proceed (y/n)?
::if /i "%Yes%" neq "y" goto end

if defined build_clean (
   if exist %build_dir% (
      echo removing %build_dir%
      rmdir %build_dir% /s /q
   ) else (
     echo "Nothing to be done for clean"
   )
   if not defined build_package ( goto end )
)

if not exist %build_dir% ( mkdir %build_dir% )
cd %build_dir%

if not defined toolset goto MSVC

:ClangCL
   echo "================================================ ClangCL ==============================================="
   echo cmake -DCMAKE_PREFIX_PATH=%QTDIR% -G %GENERATOR% -T %toolset% -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=%debug_symbol% %source_dir%
   cmake -DCMAKE_PREFIX_PATH=%QTDIR% -G %GENERATOR% -T %toolset% -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=%debug_symbol% %source_dir%
   goto endconfig:

:MSVC
  echo "================================================ MSVC ==============================================="
  ::set "build_dir=%build_root%\build-%build_arch%\%build_target%"
  echo -- build_dir    = %build_dir% --
  echo -- build_root   = %build_root% --
  echo -- build_arch   = %build_arch% --
  echo -- build_target = %build_target% --
  echo cmake -DCMAKE_PREFIX_PATH="%QTDIR%;C:/opt/netcdf-c" -G %GENERATOR% -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=%debug_symbol% %source_dir%
  set /p Yes=Proceed (y/n)?
  if /i "%Yes%" neq "y" goto end
  cmake -DCMAKE_PREFIX_PATH="%QTDIR%;C:/opt/netcdf-c" -G %GENERATOR% -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=%debug_symbol% %source_dir%

:endconfig

::echo "============ endlocal ==============" %build_dir%

endlocal && set build_dir=%build_dir%

pushd %build_dir%
echo -- %build_dir% --
echo -- You are in the %build_dir%
echo -- 'popd' command take you back to source directory.
echo --

:end
