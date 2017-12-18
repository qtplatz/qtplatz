@echo off

setlocal enableextensions
set src_dir=%HOME%\source

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
set boost_library_dir=C:\boost\x86_64\lib
set rdkit_build_dir=%build_dir%\rdkit.release

if not exist %rdkit_dir% (
   cd %src_dir%
   git clone https://github.com/rdkit/rdkit
) else (
  cd %rdkit_dir%
  git pull
)

echo mkdir %rdkit_build_dir%
mkdir %rdkit_build_dir%

if not exist %rdkit_build_dir% (
   echo No build directory %rdkit_build_dir%
   goto :EOF
)

cd %rdkit_build_dir%
@echo on

:: Due to boost::archive template class in qtplatz libraries, it is impossible to link with shared object (dll)
:: on windows.  No such restriction on Linux and Mac OSX

cmake -DBOOST_LIBRARYDIR=%boost_library_dir% ^
      -DBOOST_ROOT=%boost_root% ^
      -DBoost_USE_STATIC_LIBS=ON ^
      -DRDK_BUILD_INCHI_SUPPORT=ON ^
      -DRDK_BUILD_PYTHON_WRAPPERS=OFF ^
      -DRDK_BUILD_SWIG_JAVA_WRAPPER=OFF ^
      -DCMAKE_DEBUG_POSTFIX="d" -G %GENERATOR% %rdkit_dir%

::devenv RDKit.sln
msbuild /m:%nproc% /p:Configuration=Debug INSTALL.vcxproj
msbuild /m:%nproc% /p:Configuration=Release INSTALL.vcxproj
endlocal
cd %CWD%
