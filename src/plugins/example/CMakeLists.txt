cmake_minimum_required(VERSION 3.10)

# Remove when sharing with others.
#list(APPEND CMAKE_PREFIX_PATH "/Users/toshi/src/qt-creator/build/Qt Creator.app/Contents/Resources")
#list(APPEND CMAKE_PREFIX_PATH "/Users/toshi/src/qt-creator/build/cmake" )
#list(APPEND CMAKE_PREFIX_PATH "/Users/toshi/src/build-Darwin-arm64/qtplatz.release/cmake" )
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )

project(Example)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(QtCreator REQUIRED COMPONENTS Core)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
set( PROVIDER "MS-Cheminformatics" )
set( DESCRIPTION "Example qt-creator plugin" )

########## QtCreator 9 (Qt6) plugin.json #############
if ( QT_VERSION_MAJOR GREATER_EQUAL 6 )
  set ( DEPENDENCY_STRING "\"Dependencies\" : [\n")
  foreach ( i "Core" )
    set(_v ${IDE_VERSION})
    string ( APPEND DEPENDENCY_STRING "        { \"Name\" : \"${i}\", \"Version\" : \"${_v}\" }" )
  endforeach()
  string(APPEND DEPENDENCY_STRING "\n    ]")

  configure_file(
    "${PROJECT_SOURCE_DIR}/../servant/plugin.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json"
    )
endif()
####################### END ##########################

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  )

if ( FALSE )
  add_qtc_plugin(Example
    PLUGIN_DEPENDS
    QtCreator::Core
    DEPENDS
    Qt${QT_VERSION_MAJOR}::Widgets
    QtCreator::ExtensionSystem
    QtCreator::Utils
    adportable
    SOURCES
    .github/workflows/build_cmake.yml
    .github/workflows/README.md
    README.md
    example.cpp
    example.hpp
    example_global.hpp
    exampleconstants.hpp
    )
else()

  add_library( ${PROJECT_NAME} MODULE
    example.cpp
    example.hpp
    example_global.hpp
    exampleconstants.hpp
    )
  target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
    QtCreator::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    QtCreator::ExtensionSystem
    QtCreator::Utils
    adportable
  )

endif()

set( PROVIDER MS-Cheminformatics )
set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}
  )
