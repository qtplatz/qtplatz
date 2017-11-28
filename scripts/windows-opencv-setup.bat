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
set opencv_dir=%src_dir%\opencv
set opencv_contrib_dir=%src_dir%\opencv_contrib
set opencv_extra_dir=%src_dir%\opencv_extra
set opencv_build_dir=%build_dir%\opencv.release
set CUDA=OFF
set BUILD_CONFIG="Release"

if not exist %opencv_dir% (
   cd %src_dir%
   git clone https://github.com/opencv/opencv.git
)
if not exist %opencv_contrib_dir% (
   cd %src_dir%
   git clone https://github.com/opencv/opencv_contrib.git
)
if not exist %opencv_extra_dir% (
   cd %src_dir%
   git clone https://github.com/opencv/opencv_extra.git
)

if not exist %opencv_build_dir% (
   mkdir %opencv_build_dir%
)

@echo on
cd %opencv_build_dir%
echo opencv build directory: %cd%

copy %opencv_dir%\LICENSE %opencv_dir%\LICENSE.txt
cmake -DCMAKE_EXTRA_MODULES_PATH=%opencv_contrib_dir%\modules ^
      	  -DCMAKE_BUILD_TYPE=%BUILD_CONFIG% ^
	  -DENABLE_CXX11=ON		    ^
	  -DBUILD_PERF_TESTS=OFF            ^
	  -DWITH_XINE=ON                    ^
	  -DBUILD_TESTS=OFF                 ^
	  -DENABLE_PRECOMPILED_HEADERS=OFF  ^
	  -DCMAKE_SKIP_RPATH=ON             ^
	  -DBUILD_WITH_DEBUG_INFO=OFF       ^
	  -DCUDA_FAST_MATH=%CUDA%           ^
	  -DWITH_CUBLAS=%CUDA%              ^
	  -DCUDA_NVCC_FLAGS="--expt-relaxed-constexpr" ^
	  -DCMAKE_DEBUG_POSTFIX="d" ^
	  -DCPACK_GENERATOR=WIX ^
	  -DCPACK_WIX_UPGRADE_GUID="6C1F190B-B5A3-48A9-BA43-0B8AF0BC370E" ^
	  -DOPENCV_LICENSE_FILE="LICENSE.txt" ^
	  -DCPACK_RESOURCE_FILE_LICENSE="LICENSE.txt" ^
	  -DCPACK_WIX_LICENSE_RTF="LICENSE.txt" ^
	  -G %GENERATOR% %opencv_dir%

msbuild OpenCV.sln /t:build /m:%nproc% /p:Configuration=Debug
msbuild OpenCV.sln /t:build /m:%nproc% /p:Configuration=Release
msbuild INSTALL.vcxproj /t:build /m:%nproc% /p:Configuration=Debug
msbuild INSTALL.vcxproj /t:build /m:%nproc% /p:Configuration=Release
::msbuild PACKAGE.vcxproj /t:build /m:%nproc% /p:Configuration=Release

endlocal

cd %CWD%
