# Copyright (C) 2010-2024 Toshinobu Hondo, Ph.D.
# Copyright (C) 2013-2024 MS-Cheminformatics LLC
# Contact: toshi.hondo@qtplatz.com
cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )
project( Dataproc )

set( PROVIDER "MS-Cheminformatics" )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

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

add_definitions(
  -DDATAPROC_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
)

set ( _SOURCE surface.cpp surface.hpp )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QWT_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${QtCore_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}        # version.h
  ${CMAKE_SOURCE_DIR}/src    # app_version.h
)

# for debian 10 suport
if ( CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS_EQUAL "8.3.0" )
  set (LIBCPPFS "stdc++fs")
endif()

add_library( ${PROJECT_NAME} MODULE
  dataproc_global.hpp
  dataprocconstants.hpp
  aboutdlg.cpp                aboutdlg.hpp
  actionmanager.cpp           actionmanager.hpp
  assign_masses.cpp           assign_masses.hpp
  assign_peaks.cpp            assign_peaks.hpp
  chromatogramwnd.cpp         chromatogramwnd.hpp
  contourwnd.cpp              contourwnd.hpp
  datafolder.cpp              datafolder.hpp
  dataproceditor.cpp          dataproceditor.hpp
  dataprocessor.cpp           dataprocessor.hpp
  editorfactory.cpp           editorfactory.hpp
  documentfactory.cpp         documentfactory.hpp
  dataprocessworker.cpp       dataprocessworker.hpp
  dataprochandler.cpp         dataprochandler.hpp
  dataprocplugin.cpp          dataprocplugin.hpp
  dialogspectrometerchoice.cpp    dialogspectrometerchoice.hpp
  document.cpp                document.hpp
  elementalcompwnd.cpp        elementalcompwnd.hpp
  filepropertywidget.cpp      filepropertywidget.hpp
  ipeptidehandlerimpl.cpp     ipeptidehandlerimpl.hpp
  isnapshothandlerimpl.cpp    isnapshothandlerimpl.hpp
  mainwindow.cpp              mainwindow.hpp
  mass_calibrator.cpp         mass_calibrator.hpp
  mimetypehelper.hpp
  mode.cpp                    mode.hpp
  mscalibrationwnd.cpp        mscalibrationwnd.hpp
  mspeakswnd.cpp              mspeakswnd.hpp
  mspeaktable.cpp             mspeaktable.hpp
  msprocessingwnd.cpp         msprocessingwnd.hpp
  mspropertyform.cpp          mspropertyform.hpp
  msspectrawnd.cpp            msspectrawnd.hpp
  navigationdelegate.cpp      navigationdelegate.hpp
  navigationwidget.cpp        navigationwidget.hpp
  navigationwidgetfactory.cpp navigationwidgetfactory.hpp
  peaklist_export.cpp         peaklist_export.hpp
  qtwidgets_name.hpp
  rms_export.cpp              rms_export.hpp
  selchanged.hpp
  sessionmanager.cpp          sessionmanager.hpp
  lapdeconvdlg.cpp            lapdeconvdlg.hpp
  make_filename.cpp           make_filename.hpp
  utility.cpp                 utility.hpp
  ${_source}
  ${Resources}
)

if ( IDE_VERSION VERSION_GREATER_EQUAL "8.0.0" )
endif()

if ( WIN32 )
  set( BCRYPT "bcrypt" )
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
  ExtensionSystem
  Utils
  portfolio
  qtwrapper
  xmlparser
  Core
  Qt::Core
  ${Boost_LIBRARIES}
  ${BCRYPT}
  ${LIBCPPFS}
)

if ( APPLE )
  set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${_plugin_output_dir}"
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
