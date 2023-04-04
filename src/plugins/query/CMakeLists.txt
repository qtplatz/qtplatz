
project( Query )
set( PROVIDER MS-Cheminformatics )
if ( IDE_VERSION VERSION_LESS "9.0.0" )
  set( COMPAT_VERSION 3.2.81 )
endif()
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

########## QtCreator 9 (Qt6) plugin.json #############
if ( ${IDE_VERSION} VERSION_GREATER_EQUAL "9.0.0" )
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

if ( ${IDE_VERSION} VERSION_LESS "9.0.0" )
  configure_file(
    "${PROJECT_SOURCE_DIR}/pluginspec.in"
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
  )

  execute_process( COMMAND ${XMLPATTERNS} -no-format
    -output ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}.json
    ${QTPLATZ_SOURCE_DIR}/src/pluginjsonmetadata.xsl
    ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec
  )
endif()

add_definitions(
  -DQUERY_LIBRARY
  -D_SCL_SECURE_NO_WARNINGS
)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${CMAKE_SOURCE_DIR}/contrib/agilent/libs
  ${QtCore_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src         # version.h
  ${QWT_INCLUDE_DIRS}
  )

add_library( Query MODULE
  document.cpp
  document.hpp
  mainwindow.cpp
  mainwindow.hpp
  countingquerydialog.cpp
  countingquerydialog.hpp
  plotdialog.cpp
  plotdialog.hpp
  queryconnection.cpp
  queryconnection.hpp
  queryeditor.cpp       queryeditor.hpp
  querydocument.cpp     querydocument.hpp
  queryfactory.cpp      queryfactory.hpp
  querymode.cpp         querymode.hpp
  queryplugin.cpp       queryplugin.hpp
  queryform.cpp         queryform.hpp
  querywidget.cpp       querywidget.hpp
  queryresulttable.cpp  queryresulttable.hpp
  sqledit.cpp           sqledit.hpp
  sqlhistorydialog.cpp  sqlhistorydialog.hpp
  ${Resources}
  ${_charts_sources}
)

if ( ${IDE_VERSION} VERSION_GREATER_EQUAL "9.0.0" )
  set( EXTENSIONSYSTEM "ExtensionSystem" )
  set( UTILS "Utils" )
else()
  set( EXTENSIONSYSTEM "extensionsystem" )
  set( UTILS "utils" )
endif()


target_link_libraries(
  Query LINK_PUBLIC
  adplugin
  adcontrols
  adacquire
  adlog
  adutils
  portfolio
  adfs
  adprot
  adextension
  adplot
  adwidgets
  adportable
  chromatogr
  qtwrapper
  xmlparser
  Core
  ${EXTENSIONSYSTEM}
  ${Boost_LIBRARIES}
  ${_opencv_libraries}
  ${_charts_libraries}
)

set_target_properties( ${PROJECT_NAME}
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY_DEBUG   ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY_RELEASE ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} # for WIN32
  LIBRARY_OUTPUT_DIRECTORY ${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER} )

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER}" COMPONENT plugins
  LIBRARY DESTINATION "${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER}" COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

if ( IDE_VERSION VERSION_LESS "9.0.0" )
  install( FILES
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
    DESTINATION ${QTPLATZ_PLUGIN_DESTINATION}/${PROVIDER} COMPONENT plugins )
endif()
