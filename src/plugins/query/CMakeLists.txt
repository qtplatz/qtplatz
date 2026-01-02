
project( Query )

set ( PROVIDER MS-Cheminformatics )
set ( PLUGIN_ID "query" )
set ( PLUGIN_NAME "${PROJECT_NAME}" )
set ( PLUGIN_DISPLAY_NAME "${PROJECT_NAME}" )

set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

qtplatz_plugin_output_dir( _plugin_output_dir ${PROVIDER} )
qtplatz_plugin_install_dir( _plugin_install_dir ${PROVIDER} )

########## QtCreator 8 (Qt5/6) plugin.json #############
if ( IDE_VERSION VERSION_GREATER_EQUAL "8.0.0" )
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
  find_package(QtCreator REQUIRED COMPONENTS Core)

  set ( DEPENDENCY_STRING "\"Dependencies\" : [\n")
  string ( APPEND DEPENDENCY_STRING "        { \"Id\" : \"core\", \"Version\" : \"${IDE_VERSION}\" }" )
  string ( APPEND DEPENDENCY_STRING "\n    ]" )

  configure_file(
    "${CMAKE_SOURCE_DIR}/src/plugins/servant/plugin.json.in"
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

if ( IDE_VERSION VERSION_GREATER_EQUAL "8.0.0" )
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
