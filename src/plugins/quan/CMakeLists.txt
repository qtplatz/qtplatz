
project( Quan )

set( PROVIDER MS-Cheminformatics )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

qtplatz_plugin_output_dir( _plugin_output_dir ${PROVIDER} )
qtplatz_plugin_install_dir( _plugin_install_dir ${PROVIDER} )

########## QtCreator 8 (Qt5/6) plugin.json #############
if ( IDE_VERSION VERSION_GREATER_EQUAL "8.0.0" )
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
  find_package( QtCreator REQUIRED COMPONENTS Core )
  configure_file(
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json"
  )
endif()
####################### END ##########################

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

add_definitions(
  -DQUAN_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${QtCore_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
)

set( quan_SOURCES
  quan_global.hpp
  quanconstants.hpp
  mainwindow.cpp                mainwindow.hpp
  quanplugin.cpp                quanplugin.hpp
  quandocument.cpp              quandocument.hpp
  quaneditor.cpp                quaneditor.hpp
  quanfactory.cpp               quanfactory.hpp
  quanmode.cpp                  quanmode.hpp
  document.cpp                  document.hpp
  panelswidget.cpp              panelswidget.hpp
  paneldata.cpp                 paneldata.hpp
  doubletabwidget.cpp           doubletabwidget.hpp
  datasequencewidget.cpp        datasequencewidget.hpp
  datasequencetree.cpp          datasequencetree.hpp
  datasequencetable.cpp         datasequencetable.hpp
  compoundswidget.cpp           compoundswidget.hpp
  compoundstable.cpp            compoundstable.hpp
  findcompounds.cpp             findcompounds.hpp
  quancandidate.cpp             quancandidate.hpp
  quanconfigform.cpp            quanconfigform.hpp
  quanconfigwidget.cpp          quanconfigwidget.hpp
  quanchromatogram.cpp          quanchromatogram.hpp
  quanchromatograms.cpp         quanchromatograms.hpp
  quandatawriter.cpp            quandatawriter.hpp
  quansampleprocessor.cpp       quansampleprocessor.hpp
  quancountingprocessor.cpp     quancountingprocessor.hpp
  quanchromatogramprocessor.cpp quanchromatogramprocessor.hpp
  quanexportprocessor.cpp       quanexportprocessor.hpp
  processmethodwidget.cpp       processmethodwidget.hpp
  quanreportwidget.cpp          quanreportwidget.hpp
  quanquerywidget.cpp           quanquerywidget.hpp
  quanprocessor.cpp             quanprocessor.hpp
  quanresulttable.cpp           quanresulttable.hpp
  quanqueryform.cpp             quanqueryform.hpp
  quanconnection.cpp            quanconnection.hpp
  quanquery.cpp                 quanquery.hpp
  quanplotwidget.cpp            quanplotwidget.hpp
  quanresultwnd.cpp             quanresultwnd.hpp
  quanresultwidget.cpp          quanresultwidget.hpp
  quancmpdwidget.cpp            quancmpdwidget.hpp
  quanplotdata.cpp              quanplotdata.hpp
  quanpublisher.cpp             quanpublisher.hpp
  quanplot.cpp                  quanplot.hpp
  quansvgplot.cpp               quansvgplot.hpp
  quantarget.cpp                quantarget.hpp
  sqledit.cpp                   sqledit.hpp
  )

set( quan_RESOURCES quan.qrc )

add_library( Quan MODULE
  samplemethodform.hpp
  samplemethodform.cpp
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
  adportable
  adprot
  adprocessor
  adpublisher
  adwidgets
  chromatogr
  Core
  portfolio
  qtwrapper
  xmlparser
  ExtensionSystem
  ${w32libs} #<-- causing SEGV on Linux
  adplot
  ${Boost_LIBRARIES}
  )

if ( APPLE )
  set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}"
    )
else()
  set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${_plugin_output_dir}"
    INSTALL_RPATH "\$ORIGIN:\$ORIGIN/..:\$ORIGIN/../..:\$ORIGIN/../../..:\$ORIGIN/../../../qtplatz:${CMAKE_INSTALL_RPATH}"
    )
endif()

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${_plugin_install_dir}" COMPONENT plugins
  LIBRARY DESTINATION "${_plugin_install_dir}" COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

if ( IDE_VERSION VERSION_LESS_EQUAL "3.2.81" )
  install( FILES
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
    DESTINATION ${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER} COMPONENT plugins )
endif()
