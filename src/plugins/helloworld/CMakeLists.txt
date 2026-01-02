list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
project(HelloWorld)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(QtCreator REQUIRED COMPONENTS Core)

add_qtc_plugin(HelloWorld
  SKIP_TRANSLATION
  PLUGIN_DEPENDS Core
  SOURCES
    helloworldplugin.cpp
    helloworldtr.h
    helloworldwindow.cpp
    helloworldwindow.h
)

set( PROVIDER MS-Cheminformatics )
set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}
  )
