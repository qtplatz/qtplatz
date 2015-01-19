project( chemistry )
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

find_package( Qt5 CONFIG REQUIRED Core Network Widgets Svg Sql Script )
find_package( rdkit )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND xmlpatterns -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec
  )

find_package( qwt )

add_definitions(
  -DDATAPROC_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${RDKit_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Svg_INCLUDE_DIRS}  
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

link_directories(
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/QtProject
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

set( chemistry_SOURCES
  chemistryplugin.cpp
  chemistrymode.cpp
  moltabledelegate.cpp
  chemfile.cpp
  mainwindow.cpp
  massdefectform.cpp
  massdefectdelegate.cpp
  massdefectmethod.cpp
  moltableview.cpp
  )

set( chemistry_HEADERS
  chemistryplugin.hpp
  chemistry_global.hpp
  moltabledelegate.hpp
  chemistryconstants.hpp
  chemistrymode.hpp
  chemfile.hpp
  mainwindow.hpp
  massdefectform.hpp
  massdefectdelegate.hpp
  massdefectmethod.hpp
  moltableview.hpp
  )

add_library( ${PROJECT_NAME} MODULE
  ${chemistry_SOURCES}
  ${chemistry_HEADERS}
  ${Resources}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adutils
  adportable
  adlog
  adchem
  adcontrols
  adplugin
  qtwrapper
  adprot
  FileParsers
  GraphMol
  SmilesParse
  RDGeneral
  RDGeometryLib
  SubstructMatch
  Depictor
  Descriptors
  Core
  Qt5::Core
  Qt5::Svg
  ${Boost_LIBRARIES}
)

#LIBS *= -lboost_serialization -lboost_date_time -lboost_filesystem -lboost_system
#QT += core svg printsupport
#linux-*: LIBS += -lqwt -ldl -lboost_thread -lrt
#FORMS +=    mspropertyform.ui    dialogspectrometerchoice.ui

