cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( dataproc )

set( PROVIDER MS-Cheminformatics )

find_package( Qt5 CONFIG REQUIRED Core Widgets PrintSupport Svg Sql Network ) 

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set(COMPAT_VERSION 3.2.81)
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

configure_file(
  "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.pluginspec.in"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

configure_file(
  "${PROJECT_SOURCE_DIR}/dataproc-mimetype.xml"
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}-mimetype.xml"
  )

execute_process( COMMAND ${XMLPATTERNS} -no-format 
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
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}        # version.h
  ${CMAKE_SOURCE_DIR}/src    # app_version.h
  )

link_directories(
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz/plugins/QtProject
  ${QTPLATZ_BINARY_DIR}/lib/qtplatz  # <== qmake .lib output dir
  )

add_library( ${PROJECT_NAME} MODULE
  aboutdlg.cpp
  dataprocplugin.cpp
  mode.cpp
  mainwindow.cpp
  dataproc_document.cpp
  navigationdelegate.cpp
  navigationwidgetfactory.cpp
  navigationwidget.cpp
  dataprocessor.hpp
  actionmanager.cpp
  assign_masses.cpp
  assign_peaks.cpp
  chromatogramwnd.cpp
  mass_calibrator.cpp
  dataproceditor.cpp
  dataprocessor.cpp
  dataprocessorfactory.cpp
  dataprochandler.cpp
  elementalcompwnd.cpp
#  isequenceimpl.cpp
  isnapshothandlerimpl.cpp
  mscalibrationwnd.cpp
  mscalibspectrawnd.cpp
  msprocessingwnd.cpp
  msspectrawnd.cpp
  sessionmanager.cpp
  mspropertyform.cpp
  dataprocessworker.cpp
  dialogspectrometerchoice.cpp
  filepropertywidget.cpp
  mspeakswnd.cpp
  spectrogramwnd.cpp
  ipeptidehandlerimpl.cpp
  aboutdlg.hpp
  dataproc_global.hpp
  mimetypehelper.hpp
  selchanged.hpp
  qtwidgets_name.hpp
  dataprocplugin.hpp
  dataprocconstants.hpp
  mode.hpp
  mainwindow.hpp
  dataproc_document.hpp
  navigationwidgetfactory.hpp
  navigationwidget.hpp
  dataprocessor.hpp
  actionmanager.hpp
  chromatogramwnd.hpp
  dataproceditor.hpp
  dataprocessor.hpp
  dataprocessorfactory.hpp
  dataprochandler.hpp
  dataprocplugin.hpp
  elementalcompwnd.hpp
#  isequenceimpl.hpp
  isnapshothandlerimpl.hpp
  mscalibrationwnd.hpp
  mscalibspectrawnd.hpp
  msprocessingwnd.hpp
  msspectrawnd.hpp
  navigationdelegate.hpp
  navigationwidget.hpp
  navigationwidgetfactory.hpp
  sessionmanager.hpp
  mainwindow.hpp
  mode.hpp
  assign_masses.hpp
  mass_calibrator.hpp
  assign_peaks.hpp
  mspropertyform.hpp
  dataprocessworker.hpp
  dialogspectrometerchoice.hpp
  filepropertywidget.hpp
  mspeakswnd.hpp
  spectrogramwnd.hpp
  ipeptidehandlerimpl.hpp
  ${Resources}
  )

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adplot
  adplugin
  adcontrols
  adlog
  adutils
  adwidgets
  portfolio
  qtwrapper
  adfs
  chromatogr
  adprot
  adextension
  adportable
  adprocessor
  xmlparser
  Core
  extensionsystem
  utils
  Qt5::Core
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
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}-mimetype.xml"
  DESTINATION ${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER} COMPONENT plugins )
