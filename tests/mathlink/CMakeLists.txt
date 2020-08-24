cmake_minimum_required(VERSION 2.6.2)

project(WSTP_TEST)

set( CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/../../cmake/Modules" ${CMAKE_MODULE_PATH})

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin)  # .exe | .dll

find_package ( Wolfram )

message( STATUS "Wolfram FOUND: " ${Wolfrom_FOUND} )
message( STATUS "WSCC: " ${WSCC} )

#subdirs( addtwo formula )
subdirs( addtwo )
