cmake_minimum_required (VERSION 2.8.9)
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( formula_parser )

set(CMAKE_CXX_STANDARD 14)

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  )

add_executable( formula_parser  formula_parser.cpp )
