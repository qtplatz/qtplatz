cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( sequence )
set( PROVIDER MS-Cheminformatics )

find_package( Qt5 CONFIG REQUIRED Core Svg Sql PrintSupport Widgets Script Network WebKit WebKitWidgets )
find_package( qwt )
find_package( Boost REQUIRED COMPONENTS system date_time regex thread filesystem wserialization serialization )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
set( sequence_RESOURCES sequence.qrc )

set(COMPAT_VERSION 3.2.81)
set(VERSION 3.1.4)

configure_file(
  "${PROJECT_SOURCE_DIR}/pluginspec.in"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

configure_file(
  "${PROJECT_SOURCE_DIR}/sequence-mimetype.xml"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}-mimetype.xml"
  )


execute_process( COMMAND ${XMLPATTERNS} -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"  
  )

add_definitions(
  -DSEQUENCE_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

#QT       += xml
#LIBS += -L$$IDE_PLUGIN_PATH/QtProject
#LIBS += -l$$qtLibraryTarget( Core )

# Link order caution: adportable should be linked after adfs for utf::to_utf8 reference

#  LIBS += -lboost_system -lboost_filesystem -lboost_serialization -lboost_date_time -ldl

set( sequence_SOURCES
  sequenceplugin.cpp
  sequenceeditor.cpp
  sequencefactory.cpp
  sequencedocument.cpp
  mainwindow.cpp
  mode.cpp
  sequencewnd.cpp
  sequencedelegate.cpp
  sequenceview.cpp
  )

set( sequence_HEADERS
  sequence_global.h
  sequenceplugin.hpp
  sequencefactory.hpp
  sequencedocument.hpp
  sequenceeditor.hpp
  constants.hpp
  mainwindow.hpp
  mode.hpp
  sequencewnd.hpp
  sequencedelegate.hpp
  sequenceview.hpp
  )

add_library( sequence MODULE
  ${sequence_SOURCES}
  ${sequence_HEADERS}
  ${sequence_RESOURCES}
  )

target_link_libraries(
  sequence LINK_PUBLIC
  adcontrols
  adutils
  adplot
  acewrapper
  qtwrapper
  xmlparser
  adplugin
  adextension
  adsequence
  adwidgets
  adlog
  adfs
  adportable
  Core
  extensionsystem
  ${Boost_LIBRARIES}
  )

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} )
