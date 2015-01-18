project( chemistry )
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
  "${PROJECT_SOURCE_DIR}/dataproc-mimetype.xml"
  "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/dataproc-mimetype.xml"
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


find_package( qwt )

add_definitions(
  -DDATAPROC_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
  )

include_directories(
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${Qt5Core_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
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
  isequenceimpl.cpp
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
  isequenceimpl.hpp
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
  xmlparser
  Core
  extensionsystem
  utils
  Qt5::Core
  ${Boost_LIBRARIES}
)

#LIBS *= -lboost_serialization -lboost_date_time -lboost_filesystem -lboost_system
#QT += core svg printsupport
#linux-*: LIBS += -lqwt -ldl -lboost_thread -lrt
#FORMS +=    mspropertyform.ui    dialogspectrometerchoice.ui

