
project( peptide )

cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

find_package( Qt5 CONFIG REQUIRED Core Network Widgets Sql Script )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set(COMPAT_VERSION 3.2.81)
set(VERSION 3.1.4)

configure_file(
  "${PROJECT_SOURCE_DIR}/pluginspec.in"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND xmlpatterns -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.pluginspec"  
  )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

set( peptide_SOURCES
  peptideplugin.cpp
  mainwindow.cpp
  peptidemode.cpp 
  proteintable.cpp
  proteinwnd.cpp
  digestedpeptidetable.cpp
  )

set( peptide_HEADERS
  peptideplugin.hpp
  peptide_global.hpp
  peptideconstants.hpp
  mainwindow.hpp
  peptidemode.hpp
  proteintable.hpp
  proteinwnd.hpp
  digestedpeptidetable.hpp
  )

add_library( ${PROJECT_NAME} MODULE
  ${peptide_SOURCES}
  ${peptide_HEADERS}
  ${Resources}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adplugin
  adcontrols
  adlog
  adutils
  adfs
  adprot
  adextension
  adplot
  adportable
  qtwrapper
  Core
  extensionsystem
  utils
  Qt5::Core
  ${Boost_LIBRARIES}
)

#QT += core svg printsupport
#greaterThan( QT_MAJOR_VERSION, 4 ): QT += widgets
#PROVIDER = MS-Cheminformatics

#  LIBS += -lboost_system           -lboost_filesystem           -lboost_date_time

#linux-*: LIBS += -lqwt -ldl
#macx: QMAKE_LFLAGS+=-Wl,-search_paths_first

# peptide files

## set the QTC_SOURCE environment variable to override the setting here
#QTCREATOR_SOURCES = $$(QTC_SOURCE)

## set the QTC_BUILD environment variable to override the setting here
#IDE_BUILD_TREE = $$(QTC_BUILD)

