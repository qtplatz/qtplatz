@echo off

set BOOST_VERSION=
set QMAKE=
set BUILD_ROOT=
set SOURCE_ROOT=
set GENERATOR=
set CWD="%~dp0"
set SOURCE_DIR=%~dp0\windows

::replace '\\' to '\'
set SOURCE_DIR=%SOURCE_DIR:\\=\%

call %CWD%\constants.bat BOOST_VERSION QMAKE SOURCE_ROOT BUILD_ROOT GENERATOR

set BUILD_DIR=%SOURCE_ROOT%\build-x86_64\windows
echo "##############################################"
echo "-------- windows-bootstrap.bat ---------------"
echo "SOURCE_ROOT=%SOURCE_ROOT%"
echo "SOURCE_DIR=%SOURCE_DIR%"
echo "BOOST_VERSION"=%BOOST_VERSION%
echo "BUILD_DIR=%BUILD_DIR%"
echo "QMAKE=%QMAKE%"
echo "GENERATOR=%GENERATOR%"
echo "##############################################"

::pause
if not exist %BUILD_DIR% (
   echo mkdir "%BUILD_DIR%"
   mkdir "%BUILD_DIR%"
)

pushd %BUILD_DIR%
echo cmake -G %GENERATOR% %SOURCE_DIR%
cmake -DBOOST_VERSION=%BOOST_VERSION% ^
      -DQMAKE=%QMAKE% ^
      -G %GENERATOR% ^
      %SOURCE_DIR%

nmake help
:end
