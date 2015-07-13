:#!cmd.exe
@echo off
set source_dir=%cd%
set build_root="..\build"
set build_type=debug

for %%i in (%*) do (
    if %%i==release (
       set build_type=release
    ) else if %%i==package (
       set build_type=package
    )
)

set build_dir=%build_root%\qtplatz-x86_64.%build_type%
mkdir %build_dir%
cd %build_dir%

if %build_type%==release (
  cmake -G "Visual Studio 12 Win64" -DCMAKE_BUILD_TYPE=Release %source_dir%
) else if %build_type%==package (
  cmake -G "Visual Studio 12 Win64" -DCMAKE_BUILD_TYPE=Release %source_dir%
  cd contrib\installer\wix
) else (
  cmake -G "Visual Studio 12 Win64" -DCMAKE_BUILD_TYPE=Debug %source_dir%
)
