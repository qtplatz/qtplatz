@echo off
setlocal enableextensions

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
set opencv_dir=%src_dir%\opencv
set opencv_contrib_dir=%src_dir%\opencv_contrib
set opencv_extra_dir=%src_dir%\opencv_extra
set opencv_build_dir=%build_dir%\opencv.release
set CUDA=OFF

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


cd %opencv_build_dir%
cmake -DCMAKE_EXTRA_MODULES_PATH=%opencv_contrib_dir%\opencv_contrib\modules \
      	  -DCMAKE_BUILD_TYPE=%BUILD_CONFIG% \
	  -DENABLE_CXX11=ON		   \
	  -DBUILD_PERF_TESTS=OFF           \
	  -DWITH_XINE=ON                   \
	  -DBUILD_TESTS=OFF                \
	  -DENABLE_PRECOMPILED_HEADERS=OFF \
	  -DCMAKE_SKIP_RPATH=ON            \
	  -DBUILD_WITH_DEBUG_INFO=OFF      \
	  -DCUDA_FAST_MATH=%CUDA%          \
	  -DWITH_CUBLAS=%CUDA%             \
	  -DCUDA_NVCC_FLAGS="--expt-relaxed-constexpr" \
	  %opencv_dir%

endlocal

:devenv Opencv.sln
:msbuild /m:%nproc% /p:Configuration=Debug INSTALL.vcxproj
:msbuild /m:%nproc% /p:Configuration=Release INSTALL.vcxproj

cd %CWD%
