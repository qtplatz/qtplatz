cmake_policy( SET CMP0020 NEW )
cmake_policy( SET CMP0028 NEW )

project( figshare )

set( PROVIDER MS-Cheminformatics )

set( CMAKE_INCLUDE_CURRENT_DIR ON )
set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTOUIC ON )
set( CMAKE_AUTORCC ON )

file( GLOB UIs *.ui )
file( GLOB Resources *.qrc )

set( COMPAT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )
set( DESCRIPTION "figshare QtCreator plugin" )

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
else()
  message( FATAL_ERROR "Not supported QtCreator version: ${IDE_VERSION}" )
endif()

add_definitions( -DBOOST_CERTIFY_HEADER_ONLY=1 )

find_package( OpenSSL )
if ( OPENSSL_FOUND )
  add_definitions( -DOPENSSL_FOUND=1 -DPUBCHEM_REST=1 )
endif()

if ( MSVC )
  add_definitions(  -D_SCL_SECURE_NO_WARNINGS )
endif()

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${QWT_INCLUDE_DIRS}
  ${RDKit_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/src/plugins
  ${QtCore_INCLUDE_DIRS}
  ${QtSvg_INCLUDE_DIRS}
  "${CMAKE_SOURCE_DIR}/src"         # version.h
  "${OPENSSL_INCLUDE_DIR}"
  "${CMAKE_SOURCE_DIR}/contrib"     # boost/certify
)

add_library( ${PROJECT_NAME} MODULE
  figshareplugin.cpp figshareplugin.hpp
  mainwindow.cpp mainwindow.hpp
  restwnd.cpp restwnd.hpp
  csvwnd.cpp csvwnd.hpp
  csvwidget.cpp csvwidget.hpp
  csvtable.cpp csvtable.hpp
  document.cpp document.hpp
  ${Resources}
)

target_link_libraries( ${PROJECT_NAME} LINK_PUBLIC
  adfs
  adurl
  adutils
  adportable
  adlog
  adchem
  adcontrols
  adplugin
  adwidgets
  adprot
  qtwrapper
  pug
  Core
  Qt::Core
  Qt::Svg
  ${RDKit_LIBRARIES}
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
