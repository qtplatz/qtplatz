#echo off

if "%1"=="debug" (
  set CONFIG=Debug
  set BUILD_TARGET=Build
  set TARGET=build
) else (
  set CONFIG=Release
  set BUILD_TARGET=Build
  set TARGET=build
)

mkdir build
cd build
cmake -G "Visual Studio 12 Win64" ..

msbuild qtplatz.sln /m /t:%BUILD_TARGET% /p:Configuration=%CONFIG%
