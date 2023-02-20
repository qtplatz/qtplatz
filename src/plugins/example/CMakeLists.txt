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

set( PROVIDER MS-Cheminformatics )
set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}
  )
