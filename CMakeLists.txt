cmake_minimum_required (VERSION 2.8.11)
project (qtplatz)

set(Boost_USE_STATIC_LIBS ON)
set(BOOST_VERSION boost-1_57)

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601")
  set(Boost_INCLUDE_DIR "C:/Boost/include/${BOOST_VERSION}")
  set(Boost_LIBRARY_DIR "C:/Boost/x86_64/lib")
else()
  set(Boost_INCLUDE_DIR "/usr/local/${BOOST_VERSION}/include")
  set(Boost_LIBRARY_DIR "/usr/local/${BOOST_VERSION}/lib")
endif()

find_package(Boost 1.56 REQUIRED)

add_definitions(-DUNICODE -D_UNICODE)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)

if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(CMAKE_INSTALL_PREFIX ./bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${Boost_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src/libs )
link_directories(${Boost_LIBRARY_DIRS} ${CMAKE_BINARY_DIR}/lib)

add_subdirectory (src)
