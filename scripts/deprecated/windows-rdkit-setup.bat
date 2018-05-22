@echo off
set BOOST_VERSION=
set QMAKE=
set BUILD_ROOT=
set SOURCE_ROOT=
set GENERATOR=
set CWD="%~dp0"

setlocal enableextensions

call %CWD%\constants.bat BOOST_VERSION QMAKE SOURCE_ROOT BUILD_ROOT GENERATOR
set RDBASE=%SOURCE_ROOT%\rdkit
set RDKIT_BUILD_DIR=%BUILD_ROOT%\rdkit.release

set eigen_dir=%SOURCE_ROOT%\eigen

if not exist %eigen_dir% (
   git clone https://github.com/eigenteam/eigen-git-mirror %eigen_dir%
)

echo SOURCE_ROOT=%SOURCE_ROOT%
echo BUILD_ROOT=%BUILD_ROOT%
echo RDBASE=%RDBASE%
echo GENERATOR=%GENERATOR%

if not exist %RDBASE% (
   git clone https://github.com/rdkit/rdkit %SOURCE_ROOT%/rdkit
   git checkout Release_2018_03_1
) else (
  pushd %RDBASE%
  git pull
  git checkout Release_2018_03_1
  popd
)

echo mkdir %RDKIT_BUILD_DIR%
mkdir %RDKIT_BUILD_DIR%

if not exist %RDKIT_BUILD_DIR% (

   echo No build directory %RDKIT_BUILD_DIR%

) else (

  @echo on
  pushd %RDKIT_BUILD_DIR%

:: Due to boost::archive template class in qtplatz libraries, it is impossible to link with shared object (dll)
:: on windows.  No such restriction on Linux and OSX

   cmake -DBOOST_LIBRARYDIR=%BOOST_LIBRARY_DIR% ^
   	 -DBOOST_ROOT=%BOOST_ROOT% ^
      	 -DBoost_USE_STATIC_LIBS=ON ^
      	 -DRDK_BUILD_INCHI_SUPPORT=ON ^
      	 -DRDK_BUILD_PYTHON_WRAPPERS=OFF ^
      	 -DRDK_BUILD_SWIG_JAVA_WRAPPER=OFF ^
      	 -DRDK_INSTALL_STATIC_LIBS=ON ^
	 -DRDK_INSTALL_DYNAMIC_LIBS=ON ^
      	 -DCMAKE_DEBUG_POSTFIX="d" -G %GENERATOR% %RDBASE%

    msbuild /m:%NUMBER_OF_PROCESSORS% /p:Configuration=Debug INSTALL.vcxproj
    msbuild /m:%NUMBER_OF_PROCESSORS% /p:Configuration=Release INSTALL.vcxproj
)

endlocal

cd %CWD%
