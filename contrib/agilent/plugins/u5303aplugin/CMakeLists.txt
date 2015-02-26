cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( u5303aplugin )

set( PROVIDER MS-Cheminformatics )

#QT += core svg printsupport widgets
find_package( Qt5 CONFIG REQUIRED Core Widgets Sql Svg Script Network )
find_package( qwt )
find_package( ace+tao )
find_package( Boost REQUIRED COMPONENTS system date_time regex filesystem wserialization serialization )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec.in"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND xmlpatterns -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec
  )

add_definitions(
  -DU5303A_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  )

#include(../../agilentplugin.pri)
#nclude(../../qwt.pri)
#include(../../boost.pri)

#INCLUDEPATH *= $$OUT_PWD/../../libs
include_directories(
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${CMAKE_BINARY_DIR}/src/libs/adinterface
  ${CMAKE_BINARY_DIR}/src/libs
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${ACE+TAO_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

# u5303 files

add_library(
  u5303aplugin MODULE
  u5303aplugin.cpp
  mainwindow.cpp
  u5303amode.cpp
  document.cpp
  waveformwnd.cpp
  u5303aform.cpp
  u5303amethodtable.cpp
  u5303amethodwidget.cpp
  isequenceimpl.cpp
  u5303aplugin.hpp
  u5303a_global.hpp
  u5303a_constants.hpp
  mainwindow.hpp
  u5303amode.hpp
  document.hpp
  waveformwnd.hpp
  u5303aform.hpp
  u5303amethodtable.hpp
  u5303amethodwidget.hpp
  isequenceimpl.cpp
  )

target_link_libraries(
  u5303aplugin LINK_PUBLIC
  adfs
  adutils
  adcontrols
  adinterface
  adplot
  adwidgets
  acewrapper
  adplugin
  adextension
  adorbmgr
  adlog
  adportable
  xmlparser
  Core
  u5303a
  u5303aspectrometer
  ${ACE+TAO_LIBRARIES}
  ${Boost_LIBRARIES}
  )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} )
  
#  LIBS += -l$$qtLibraryTarget(adcontrols) \
#        -l$$qtLibraryTarget(adlog) \
#        -l$$qtLibraryTarget(adplugin) \
#        -l$$qtLibraryTarget(adfs) \
#        -l$$qtLibraryTarget(adextension) \
#        -l$$qtLibraryTarget(adplot) \
#        -l$$qtLibraryTarget(adportable) \
#        -l$$qtLibraryTarget(adwidgets) \
#        -l$$qtLibraryTarget(qtwrapper) \
#        -l$$qtLibraryTarget(u5303a) \
#        -l$$qtLibraryTarget(u5303aspectrometer)

# Qt Creator linking

## set the QTC_SOURCE environment variable to override the setting here
#QTCREATOR_SOURCES = $$(QTC_SOURCE)

## set the QTC_BUILD environment variable to override the setting here
#IDE_BUILD_TREE = $$(QTC_BUILD)

#    u5303a.qrc
#    u5303aform.ui
