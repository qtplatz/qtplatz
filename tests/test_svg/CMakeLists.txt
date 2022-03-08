cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0042 NEW )

project( test_svg )

if ( NOT Boost_FOUND ) # -- standalone
  cmake_minimum_required( VERSION 3.22 )
  set ( CMAKE_EXPORT_COMPILE_COMMANDS ON )

  if ( WIN32 )
    list( APPEND CMAKE_MODULE_PATH "C:/qtplatz"    "C:/opt/Eigen3/share/eigen3/cmake/")
    list( APPEND CMAKE_PREFIX_PATH "C:/opt/Eigen3" "C:/opt/RDKit/lib/cmakec/rdkit")
  elseif( APPLE )
    find_path( qtplatz_config_DIR NAMES "qtplatz-config.cmake"
      HINTS "$ENV{HOME}/src/build-Darwin-${CMAKE_SYSTEM_PROCESSOR}/qtplatz.release" )
    list( APPEND CMAKE_PREFIX_PATH ${qtplatz_config_DIR} )
  else()
    list( APPEND CMAKE_MODULE_PATH "/opt/qtplatz")
  endif()

  set( Boost_NO_WARN_NEW_VERSIONS ON )
  find_package( qtplatz REQUIRED )
  find_package( Boost 1.75 REQUIRED COMPONENTS
    filesystem
    json
    program_options
    system
    )

  find_program ( QMAKE qmake HINTS "/opt/Qt/5.15.2/bin" "/opt/clang_64/Qt/5.15.2/bin" )
  if ( QMAKE_FOUND )
    execute_process( COMMAND ${QMAKE} -query QT_HOST_PREFIX OUTPUT_VARIABLE QTDIR )
  endif()
  list( APPEND CMAKE_PREFIX_PATH ${QTDIR} )

  set( CMAKE_CXX_STANDARD 20 )

endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package( Qt5 CONFIG REQUIRED Core Widgets Svg Sql PrintSupport OpenGL Multimedia )

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src/libs
  "${QTPLATZ_SOURCE_DIR}/src/libs/xmlparser/pugixml-1.12/src"
  ${Qt5Svg_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  )

add_executable( ${PROJECT_NAME}
  main.cpp
  mainwindow.cpp
  mainwindow.hpp
  document.cpp
  document.hpp
  outputwidget.cpp
  outputwidget.hpp
  molgridwnd.cpp
  molgridwnd.hpp
  )

target_link_libraries( ${PROJECT_NAME}
  ${Boost_LIBRARIES}
  Qt5::Svg
  Qt5::Widgets
  Qt5::Core
  adportable
  adfs
  adui
  adlog
  qtwrapper
  xmlparser
  )
