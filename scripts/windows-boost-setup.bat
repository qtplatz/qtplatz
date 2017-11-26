@echo off
set BOOST=boost_1_65_1
set SRC=%USERPROFILE%\source
set CWD=%cd%

set NPROC=%NUMBER_OF_PROCESSORS%
set boost_dir=%SRC%\%BOOST%
set bzip2_dir=%SRC%\bzip2-1.0.6

echo "Visual Studio Version: " %VisualStudioVersion%
echo "BOOST: " %boost_dir%
echo "BZIP2: " %bzip2_dir%

if %VisualStudioVersion% EQU 14.0 (
   set msvc=msvc-14.0
)
if %VisualStudioVersion% EQU 15.0 (
   set msvc=msvc-14.1      
)

cd %boost_dir%

call bootstrap.bat

@echo on

b2 -j%nproc% toolset=%msvc% architecture=x86 address-model=64 -s BZIP2_SOURCE=%bzip2_dir% threading=multi runtime-link=shared --build-type=minimal link=static --stagedir=stage/x86_x64 stage

b2 -j%nproc% toolset=%msvc% architecture=x86 address-model=64 -s BZIP2_SOURCE=%bzip2_dir% threading=multi runtime-link=shared --build-type=minimal link=shared --stagedir=stage/x86_x64 stage install

cd %CWD%

