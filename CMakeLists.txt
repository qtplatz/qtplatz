#
cmake_minimum_required (VERSION 2.8.9)

project (adtoolkit)

include (qtplatz.cmake)

set(CMAKE_DEBUG_POSTFIX "d")

add_subdirectory (src)
add_subdirectory (contrib)
