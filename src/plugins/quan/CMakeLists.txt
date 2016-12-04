cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( Quan )
set( PROVIDER MS-Cheminformatics )
set( COMPAT_VERSION 3.2.81 )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

find_package( Qt5 CONFIG REQUIRED Core Svg Sql PrintSupport Widgets Network )

#if ( "${Qt5_VERSION}" VERSION_LESS "5.6" )
#  find_package( Qt5 CONFIG REQUIRED WebKitWidgets )
#else()
#  find_package( Qt5 CONFIG REQUIRED WebEngineWidgets )
#endif()

find_package( qwt )
find_package( Boost REQUIRED COMPONENTS chrono system date_time regex thread filesystem wserialization serialization )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

configure_file(
  "${PROJECT_SOURCE_DIR}/pluginspec.in"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

execute_process( COMMAND ${XMLPATTERNS} -no-format 
  -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec  
  )

add_definitions(
  -DQUAN_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  )

link_directories(
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

set( quan_SOURCES
  mainwindow.cpp
  quanplugin.cpp
  quaneditor.cpp
  quanfactory.cpp
  quanmode.cpp
  quandocument.cpp
  panelswidget.cpp
  paneldata.cpp
  doubletabwidget.cpp
  datasequencewidget.cpp
  datasequencetree.cpp
  datasequencetable.cpp  
  compoundswidget.cpp
  compoundstable.cpp
  quancandidate.cpp
  quanconfigform.cpp
  quanconfigwidget.cpp
  quanchromatogram.cpp  
  quanchromatograms.cpp  
  quandatawriter.cpp
  quansampleprocessor.cpp
  quanchromatogramsprocessor.cpp
  processmethodwidget.cpp
  quanreportwidget.cpp
  quanquerywidget.cpp
  quanprocessor.cpp
  quanresulttable.cpp
  quanqueryform.cpp
  quanconnection.cpp
  quanquery.cpp
  quanplotwidget.cpp
  quanresultwnd.cpp
  quanresultwidget.cpp
  quancmpdwidget.cpp
  quanplotdata.cpp
  quanpublisher.cpp
  quanplot.cpp
  quansvgplot.cpp
  quantarget.cpp
  quantarget.hpp
  sqledit.cpp
  sqledit.hpp
  )

set( quan_HEADERS
  mainwindow.hpp
  quanplugin.hpp
  quan_global.hpp
  quanconstants.hpp
  quaneditor.hpp
  quanfactory.hpp
  quanmode.hpp
  quandocument.hpp
  panelswidget.hpp
  paneldata.hpp
  doubletabwidget.hpp
  datasequencewidget.hpp
  datasequencetree.hpp
  datasequencetable.hpp
  compoundswidget.hpp
  compoundstable.hpp
  quanconfigform.hpp
  quanconfigwidget.hpp
  quancandidate.hpp
  quanchromatogram.hpp  
  quanchromatograms.hpp
  quanchromatogramsprocessor.hpp
  quandatawriter.hpp
  quansampleprocessor.hpp
  processmethodwidget.hpp
  quanreportwidget.hpp
  quanquerywidget.hpp
  quanprocessor.hpp
  quanresulttable.hpp
  quanqueryform.hpp
  quanconnection.hpp
  quanquery.hpp
  quanplotwidget.hpp
  quanresultwidget.hpp
  quanresultwnd.hpp
  quancmpdwidget.hpp
  quanplotdata.hpp
  quanpublisher.hpp
  quanprogress.hpp
  quanplot.hpp
  quansvgplot.hpp
  )

set( quan_RESOURCES quan.qrc )
set( quan_FORMS doubletabwidget.ui  quanconfigform.ui  quanqueryform.ui )

add_library( Quan MODULE
  ${quan_SOURCES}
  ${quan_HEADERS}
  ${quan_RESOURCES}
  )

target_link_libraries(
  Quan LINK_PUBLIC
  adplugin
  adcontrols
  adlog
  adutils
  portfolio
  adfs
  adprot
  adextension
  adplot
  adwidgets
  adportable
  adprocessor
  adpublisher
  chromatogr
  qtwrapper
  xmlparser
  Core
  extensionsystem
  ${QWT_LIBRARIES}
  ${Boost_LIBRARIES}
  )
  
#linux-*: LIBS += -lqwt -ldl -lrt
#macx: QMAKE_LFLAGS+=-Wl,-search_paths_first

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  CXX_STANDARD 14
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  LIBRARY DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

install( FILES
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  DESTINATION ${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER} COMPONENT plugins )
