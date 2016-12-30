cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( Quan )
set( PROVIDER MS-Cheminformatics )
set( COMPAT_VERSION 3.2.81 )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

find_package( Qt5 CONFIG REQUIRED Core Svg Sql PrintSupport Widgets Network )

find_package( qwt )

if ( NOT Boost_FOUND )
  #find_package( Boost REQUIRED COMPONENTS chrono system date_time regex thread filesystem wserialization serialization )
endif()

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
  quan_global.hpp
  quanconstants.hpp
  mainwindow.cpp
  mainwindow.hpp
  quanplugin.cpp
  quanplugin.hpp
  quaneditor.cpp
  quaneditor.hpp
  quanfactory.cpp
  quanfactory.hpp
  quanmode.cpp
  quanmode.hpp
  quandocument.cpp
  quandocument.hpp
  panelswidget.cpp
  panelswidget.hpp
  paneldata.cpp
  paneldata.hpp
  doubletabwidget.cpp
  doubletabwidget.hpp
  datasequencewidget.cpp
  datasequencewidget.hpp
  datasequencetree.cpp
  datasequencetree.hpp
  datasequencetable.cpp  
  datasequencetable.hpp
  compoundswidget.cpp
  compoundswidget.hpp
  compoundstable.cpp
  compoundstable.hpp
  quancandidate.cpp
  quancandidate.hpp
  quanconfigform.cpp
  quanconfigform.hpp
  quanconfigwidget.cpp
  quanconfigwidget.hpp
  quanchromatogram.cpp  
  quanchromatogram.hpp  
  quanchromatograms.cpp  
  quanchromatograms.hpp
  quandatawriter.cpp
  quandatawriter.hpp
  quansampleprocessor.cpp
  quansampleprocessor.hpp
  quancountingprocessor.cpp
  quancountingprocessor.hpp  
  quanchromatogramsprocessor.cpp
  quanchromatogramsprocessor.hpp
  processmethodwidget.cpp
  processmethodwidget.hpp
  quanreportwidget.cpp
  quanreportwidget.hpp
  quanquerywidget.cpp
  quanquerywidget.hpp
  quanprocessor.cpp
  quanprocessor.hpp
  quanresulttable.cpp
  quanresulttable.hpp
  quanqueryform.cpp
  quanqueryform.hpp
  quanconnection.cpp
  quanconnection.hpp
  quanquery.cpp
  quanquery.hpp
  quanplotwidget.cpp
  quanplotwidget.hpp
  quanresultwnd.cpp
  quanresultwnd.hpp
  quanresultwidget.cpp
  quanresultwidget.hpp
  quancmpdwidget.cpp
  quancmpdwidget.hpp
  quanplotdata.cpp
  quanplotdata.hpp
  quanpublisher.cpp
  quanpublisher.hpp
  quanplot.cpp
  quanplot.hpp
  quansvgplot.cpp
  quansvgplot.hpp
  quantarget.cpp
  quantarget.hpp
  sqledit.cpp
  sqledit.hpp
  quanprogress.hpp
  )

set( quan_RESOURCES quan.qrc )
set( quan_FORMS doubletabwidget.ui  quanconfigform.ui  quanqueryform.ui )

add_library( Quan MODULE
  ${quan_SOURCES}
  ${quan_RESOURCES}
  )

if ( WIN32 )
  set( w32libs ${QWT_LIBRARIES} )
else()
  set( w32libs "" )
endif()

target_link_libraries(
  Quan LINK_PUBLIC
  adplugin
  adcontrols
  adlog
  adutils
  adextension
  adfs
  adprot
  adportable
  adprocessor
  adpublisher
  adwidgets
  chromatogr
  Core
  portfolio
  qtwrapper
  xmlparser
  extensionsystem
  ${w32libs} #<-- causing SEGV on Linux
  adplot
  ${Boost_LIBRARIES}
  )
  
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
