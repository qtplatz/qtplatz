# Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
# Copyright (C) 2013-2023 MS-Cheminformatics LLC
# Contact: toshi.hondo@qtplatz.com

project(SDFViewer)

set( PROVIDER MS-Cheminformatics )
set( CMAKE_INCLUDE_CURRENT_DIR ON )

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

file( GLOB UIs *.ui )
file( GLOB Resources *.qrc )

#list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
#find_package(QtCreator REQUIRED COMPONENTS Core)
#find_package(Qt5 COMPONENTS SvgWidgets QUIET)
#if (TARGET Qt5::SvgWidgets)
#  set(SVG_WIDGETS Qt5::SvgWidgets)
#endif()

########## QtCreator 8 (Qt5/6) plugin.json #############
if ( IDE_VERSION VERSION_GREATER_EQUAL "8.0.0" )
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
  find_package( QtCreator REQUIRED COMPONENTS Core )
  configure_file(
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json"
  )
else()
  message( FATAL "QtCreator version not matched" )
endif()
####################### END ##########################

add_definitions(
  -DSDFVIEWER_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${QtCore_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}        # version.h
)

add_library( ${PROJECT_NAME} MODULE
  sdfviewerplugin.cpp   sdfviewerplugin.hpp
  sdfviewerfactory.cpp  sdfviewerfactory.hpp
  sdfviewerdocument.cpp sdfviewerdocument.hpp
  sdfviewer.cpp         sdfviewer.hpp
  sdfview.cpp           sdfview.hpp
)

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adwidgets
  ExtensionSystem
  Utils
  Core
  Qt::Core
  ${Boost_LIBRARIES}
)

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}
)

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER}" COMPONENT plugins
  LIBRARY DESTINATION "${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER}" COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries
  )

#target_link_libraries( SDFViewer LINK_PUBLIC adportable )
