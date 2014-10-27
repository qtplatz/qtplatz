cmake_minimum_required (VERSION 2.8.11)
project (qtplatz)

set(Boost_USE_STATIC_LIBS ON)

if(${WIN32})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0601")
  set(Boost_INCLUDE_DIR "C:/Boost/include/boost-1_56")
  set(Boost_LIBRARY_DIR "C:/Boost/x86_64/lib")
else()
  set(Boost_INCLUDE_DIR "/usr/local/boost-1_56/include")
  set(Boost_LIBRARY_DIR "/usr/local/boost-1_56/lib")
endif()

find_package(Boost 1.56 REQUIRED)

add_definitions(-DUNICODE -D_UNICODE)

if(CMAKE_COMPILER_IS_GNUCXX)
  add_definitions(-std=c++11 -g -O2)
endif()

set(CMAKE_INSTALL_PREFIX ./bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS} ${CMAKE_BINARY_DIR}/lib)

add_subdirectory (src)
