
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

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.pluginspec"
  )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.config"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.config"
  )

execute_process( COMMAND xmlpatterns -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec
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

LIBS += -l$$qtLibraryTarget(adplugin) \
        -l$$qtLibraryTarget(adcontrols) \
        -l$$qtLibraryTarget(adlog) \
        -l$$qtLibraryTarget(adutils) \
        -l$$qtLibraryTarget(portfolio) \
        -l$$qtLibraryTarget(adfs) \
        -l$$qtLibraryTarget(adprot) \
        -l$$qtLibraryTarget(adextension) \
        -l$$qtLibraryTarget(adplot) \
        -l$$qtLibraryTarget(adportable) \
        -l$$qtLibraryTarget(qtwrapper)

!win32 {
  LIBS += -lboost_system \
          -lboost_filesystem \
          -lboost_date_time
}

#linux-*: LIBS += -lqwt -ldl
#macx: QMAKE_LFLAGS+=-Wl,-search_paths_first

# peptide files

## set the QTC_SOURCE environment variable to override the setting here
#QTCREATOR_SOURCES = $$(QTC_SOURCE)

## set the QTC_BUILD environment variable to override the setting here
#IDE_BUILD_TREE = $$(QTC_BUILD)



