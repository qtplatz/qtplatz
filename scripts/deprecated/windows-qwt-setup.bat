@echo off

set BOOST_VERSION=
set QMAKE=
set BUILD_ROOT=
set SOURCE_ROOT=
set GENERATOR=
set CWD="%~dp0"

call %CWD%\constants.bat BOOST_VERSION QMAKE SOURCE_ROOT BUILD_ROOT GENERATOR

set qwt_build_dir=%SOURCE_ROOT%\qwt-6.1

IF NOT EXIST "%qwt_build_dir%" (
   echo "## '%qwt_build_dir%' does not exist-- getting sournce from subversion"
   pushd %SOURCE_ROOT%
   svn checkout svn://svn.code.sf.net/p/qwt/code/branches/qwt-6.1
   ren qwtconfig.pri qwtconfig.pri.orig   
   popd
) else (
  echo "# found %qwt_build_dir%"
)

echo "-------------------------------"
pushd %qwt_build_dir%
sed 's/\([ \t]*QWT_CONFIG.*QwtDesigner\)/#\1/; s/\([ \t]*QWT_CONFIG.*QwtDll\)/#\1/' qwtconfig.pri.orig > qwtconfig.pri
%QMAKE% qwt.pro
nmake
nmake install
popd

:end
