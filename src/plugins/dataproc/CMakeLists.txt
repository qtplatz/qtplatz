cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( dataproc )

set( PROVIDER MS-Cheminformatics )
#find_package( Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core Widgets PrintSupport Svg Sql )

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
  -output ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.json
  ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
  ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec
  )

find_package( qwt )

add_definitions(
  -DDATAPROC_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
)

set ( _SOURCE surface.cpp surface.hpp )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${QtCore_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}        # version.h
  ${CMAKE_SOURCE_DIR}/src    # app_version.h
  )

add_library( ${PROJECT_NAME} MODULE
  aboutdlg.cpp
  aboutdlg.hpp
  actionmanager.cpp
  actionmanager.hpp
  assign_masses.cpp
  assign_masses.hpp
  assign_peaks.cpp
  assign_peaks.hpp
  chromatogramwnd.cpp
  chromatogramwnd.hpp
  contourwnd.cpp
  contourwnd.hpp
  datafolder.cpp
  datafolder.hpp
  dataproc_global.hpp
  dataprocconstants.hpp
  dataproceditor.cpp
  dataproceditor.hpp
  dataprocessor.cpp
  dataprocessor.hpp
  dataprocessor.hpp
  dataprocessor.hpp
  dataprocessorfactory.cpp
  dataprocessorfactory.hpp
  dataprocessworker.cpp
  dataprocessworker.hpp
  dataprochandler.cpp
  dataprochandler.hpp
  dataprocplugin.cpp
  dataprocplugin.hpp
  dataprocplugin.hpp
  dialogspectrometerchoice.cpp
  dialogspectrometerchoice.hpp
  document.cpp
  document.hpp
  elementalcompwnd.cpp
  elementalcompwnd.hpp
  filepropertywidget.cpp
  filepropertywidget.hpp
  ipeptidehandlerimpl.cpp
  ipeptidehandlerimpl.hpp
  isnapshothandlerimpl.cpp
  isnapshothandlerimpl.hpp
  mainwindow.cpp
  mainwindow.hpp
  mainwindow.hpp
  mass_calibrator.cpp
  mass_calibrator.hpp
  mimetypehelper.hpp
  mode.cpp
  mode.hpp
  mode.hpp
  mscalibrationwnd.cpp
  mscalibrationwnd.hpp
  mspeakswnd.cpp
  mspeakswnd.hpp
  mspeaktable.cpp
  mspeaktable.hpp
  msprocessingwnd.cpp
  msprocessingwnd.hpp
  mspropertyform.cpp
  mspropertyform.hpp
  msspectrawnd.cpp
  msspectrawnd.hpp
  navigationdelegate.cpp
  navigationdelegate.hpp
  navigationwidget.cpp
  navigationwidget.hpp
  navigationwidget.hpp
  navigationwidgetfactory.cpp
  navigationwidgetfactory.hpp
  navigationwidgetfactory.hpp
  peaklist_export.cpp
  peaklist_export.hpp
  qtwidgets_name.hpp
  rms_export.cpp
  rms_export.hpp
  selchanged.hpp
  sessionmanager.cpp
  sessionmanager.hpp
  lapdeconvdlg.cpp
  lapdeconvdlg.hpp
  make_filename.cpp
  make_filename.hpp
  utility.cpp
  utility.hpp
  ${_source}
  ${Resources}
  )

if ( WIN32 )
  set( BCRYPT "bcrypt" )
endif()
if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL "8.3.0" )
  set (LIBCPPFS "stdc++fs")
endif()

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adcontrols
  adextension
  adfs
  adio
  adlog
  admtwidgets
  adplot
  adplugin
  adportable
  adprocessor
  adprot
  adpublisher
  adutils
  adwidgets
  chromatogr
  extensionsystem
  portfolio
  qtwrapper
  utils
  xmlparser
  Core
  Qt::Core
  ${Boost_LIBRARIES}
  ${BCRYPT}
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
  "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}-mimetype.xml"
  DESTINATION ${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER} COMPONENT plugins )
