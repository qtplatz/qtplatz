:#!cmd.exe
@echo off
set source_dir=%cd%
set build_root=..\build-x86_64
set build_type=release
set build_tests=false
set build_clean=false
set tools=vc12

for %%i in (%*) do (
    if %%i==release (
       set build_type=release
    ) else if %%i==package (
       set build_type=package
    ) else if %%i==tests (
       set build_tests=true
    ) else if %%i==clean (
       set build_clean=true
    ) else if %%i==vc14 (
       set tools=vc14
    )
)

set build_dir=%build_root%\qtplatz.%build_type%
if %tools%==vc14 (
   set build_dir=%build_root%\qtplatz.%build_type%-vc14
)

if %build_clean%==true (
  echo rmdir %build_dir% /s /q
  rmdir %build_dir% /s /q
  goto end
)

mkdir %build_dir%
cd %build_dir%

if %tools%==vc12 (

   if %build_type%==release (
     cmake -G "Visual Studio 12 Win64" -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=ON %source_dir%
   ) else if %build_type%==package (
     cmake -G "Visual Studio 12 Win64" -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=OFF %source_dir%
     cd contrib\installer\wix
     nmake help
     goto end
   ) else (
     cmake -G "Visual Studio 12 Win64" -DCMAKE_BUILD_TYPE=Debug -DDEBUG_SYMBOL:BOOL=ON %source_dir%
   )
   if %build_tests%==true (
      echo "Build tests directory on %cd%"
      mkdir tests
      cd tests
      cmake -G "Visual Studio 12 Win64" %source_dir%\tests
   )
   
) else if %tools%==vc14 (

  if %build_type%==release (
     cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=ON %source_dir%
  ) else if %build_type%==package (
     cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Release -DDEBUG_SYMBOL:BOOL=OFF %source_dir%
     cd contrib\installer\wix
     nmake help
  ) else (
     cmake -G "Visual Studio 14 2015 Win64" -DCMAKE_BUILD_TYPE=Debug %source_dir%
  )
)

cd %source_dir%

:end
