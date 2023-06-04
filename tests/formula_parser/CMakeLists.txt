cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( formula_parser )

set(CMAKE_CXX_STANDARD 14)

include_directories(
  ${OpenCL_INCLUDE_DIRS}
  )

add_executable( formula_parser  formula_parser.cpp )
