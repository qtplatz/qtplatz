cmake_minimum_required (VERSION 3.18.2)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/libs )

add_subdirectory( adplugins )
add_subdirectory( libs )
add_subdirectory( plugins )
