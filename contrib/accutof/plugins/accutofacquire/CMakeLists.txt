cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( accutofacquire )

set( PROVIDER MS-Cheminformatics )
set( COMPAT_VERSION 3.2.81 )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

########## QtCreator 9 (Qt6) plugin.json #############
if ( QT_VERSION_MAJOR GREATER_EQUAL 6 )
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
  find_package(QtCreator REQUIRED COMPONENTS Core)

  set ( DEPENDENCY_STRING "\"Dependencies\" : [\n")
  foreach ( i "Core" )
    set(_v ${IDE_VERSION})
    string ( APPEND DEPENDENCY_STRING "        { \"Name\" : \"${i}\", \"Version\" : \"${_v}\" }" )
  endforeach()
  string(APPEND DEPENDENCY_STRING "\n    ]")
  configure_file(
    "${PROJECT_SOURCE_DIR}/accutofacquire.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json"
    )
endif()
####################### END ##########################

#QT += core svg printsupport widgets
#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core Widgets Sql Svg Network PrintSupport )
find_package( qwt )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

configure_file(
  "${QTPLATZ_SOURCE_DIR}/src/plugins/pluginspec.in"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND ${XMLPATTERNS} -no-format
  -output ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec
  )

add_definitions(
  -DACCUTOFACQUIRE_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  -DBOOST_ASIO_NO_DEPRECATED
  -DXCHROMATOGRAMSMETHOD=1
  -DTOFCHROMATOGRAMSMETHOD=0
  )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QTPLATZ_SOURCE_DIR}/contrib/agilent/libs
  ${QTPLATZ_SOURCE_DIR}/contrib/accutof/libs
  ${QTPLATZ_SOURCE_DIR}/src/libs
  ${QTPLATZ_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/src/libs
  ${QWT_INCLUDE_DIRS}
  ${QtWidgets_INCLUDE_DIRS}
  ${QtCore_INCLUDE_DIRS}
  ${QTPLATZ_SOURCE_DIR}/src         # version.h
  )

find_package( Threads REQUIRED )

add_library(
  ${PROJECT_NAME} MODULE
  constants.cpp
  constants.hpp
  document.cpp
  document.hpp
  iu5303afacade.cpp
  iu5303afacade.hpp
  isequenceimpl.cpp
  mainwindow.cpp
  mainwindow.hpp
  pkdavgwriter.cpp
  pkdavgwriter.hpp
  resultwriter.cpp
  resultwriter.hpp
  task.cpp
  task.hpp
  mode.hpp
  mode.cpp
  accutofacquireplugin.cpp
  accutofacquireplugin.hpp
  waveformwnd.cpp
  waveformwnd.hpp
  isequenceimpl.cpp
  isequenceimpl.hpp
  moleculeswidget.cpp
  moleculeswidget.hpp
  ${Resources}
  )

if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL "8.3.0" )
  set (LIBCPPFS "stdc++fs")
endif()

target_link_libraries(
  ${PROJECT_NAME} LINK_PUBLIC
  acewrapper
  acqrswidgets
  adacquire
  adacquire
  adcontrols
  adextension
  adfs
  adlog
  adplot
  adplugin
  adplugin_manager
  adportable
  adurl
  adutils
  adwidgets
  xmlparser
  Core
  ExtensionSystem
  Utils
  Qt::Svg
  Qt::Widgets
  acqrscontrols
  #  u5303a
  qtwrapper
  ${QWT_LIBRARIES}
  ${Boost_LIBRARIES}
  Threads::Threads
  ${LIBCPPFS}
  )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  LIBRARY DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

install( FILES
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )
