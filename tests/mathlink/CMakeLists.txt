cmake_minimum_required(VERSION 2.6.2)

project(MathLink)

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../../cmake/Modules" ${CMAKE_MODULE_PATH})

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin)  # .exe | .dll

find_package (MathLink 10.4 )

subdirs( addtwo formula )
