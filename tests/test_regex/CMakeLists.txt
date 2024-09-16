cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( test_regex )

set( CMAKE_CXX_STANDARD 20 )

add_executable( ${PROJECT_NAME} main.cpp )
