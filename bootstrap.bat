:#!cmd.exe
@echo off
set source_dir=%cd%
set build_root="..\build"
set build_type=debug
set build_tests=false

for %%i in (%*) do (
    if %%i==release (
       set build_type=release
    ) else if %%i==package (
       set build_type=package
    ) else if %%i==tests (
       set build_tests=true
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

if %build_tests%==true (
   echo "Build tests directory on %cd%"
   mkdir tests
   cd tests
   cmake -G "Visual Studio 12 Win64" %source_dir%\tests
   cd ..
)


