@echo off

if %VisualStudioVersion% EQU 14.0 (
   set GENERATOR="Visual Studio 14 2015 Win64"
   set build_dir=%src_dir%\build-vc14-x86_64
)
if %VisualStudioVersion% EQU 15.0 (
   set GENERATOR="Visual Studio 15 2017 Win64"
   set build_dir=%src_dir%\build-vc15-x86_64
)

set CWD=%cd%
set nproc=%NUMBER_OF_PROCESSORS%
set src_dir=%HOME%\source
set rdkit_dir=%src_dir%\rdkit
set boost_root=C:\boost\include\boost-1_65_1
set boost_librarydir=C:\boost\x86_64\lib
set rdkit_build_dir=%build_dir%\rdkit.release

if not exist %rdkit_dir% (
   cd %src_dir%
   git clone https://github.com/rdkit/rdkit
) else (
  cd %rdkit_dir%
  git pull
)

setlocal enableextensions
mkdir %rdkit_build_dir%
endlocal

cd %rdkit_build_dir%
@echo on
::cmake -DBOOST_LIBRARYDIR=%boost_library_dir% -DBOOST_ROOT=%boost_root% -DRDK_BUILD_INCHI_SUPPORT=ON -DRDK_BUILD_PYTHON_WRAPPERS=OFF -DCMAKE_DEBUG_POSTFIX="d" -G "Visual Studio 15 2017 Win64" %rdkit_dir%

cmake -DBOOST_LIBRARYDIR=%boost_library_dir% -DBOOST_ROOT=%boost_root% -DRDK_BUILD_INCHI_SUPPORT=ON -DRDK_BUILD_PYTHON_WRAPPERS=OFF -DBoost_USE_STATIC_LIBS=ON -DCMAKE_DEBUG_POSTFIX="d" -G %GENERATOR% %rdkit_dir%

:devenv RDKit.sln
msbuild /m:%nproc% /p:Configuration=Debug INSTALL.vcxproj
msbuild /m:%nproc% /p:Configuration=Release INSTALL.vcxproj

cd %CWD%
