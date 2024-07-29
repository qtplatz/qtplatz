#####
# chemistry/CMakeLists.txt
project( Chemistry )

set( PROVIDER MS-Cheminformatics )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB UIs *.ui)
file(GLOB Resources *.qrc)

set( DESCRIPTION "Chemistry plugin" )
set( VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

qtplatz_plugin_output_dir( _plugin_output_dir ${PROVIDER} )
qtplatz_plugin_install_dir( _plugin_install_dir ${PROVIDER} )

########## QtCreator 8 (Qt5/6) plugin.json #############
if ( IDE_VERSION VERSION_GREATER_EQUAL "8.0.0" )
  list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/cmake" )
  find_package( QtCreator REQUIRED COMPONENTS Core )

  set ( DEPENDENCY_STRING "\"Dependencies\" : [\n")
  foreach ( i "Core" )
    set(_v ${IDE_VERSION})
    string ( APPEND DEPENDENCY_STRING "        { \"Name\" : \"${i}\", \"Version\" : \"${_v}\" }" )
  endforeach()
  string(APPEND DEPENDENCY_STRING "\n    ]")

  configure_file(
    "${PROJECT_SOURCE_DIR}/../servant/plugin.json.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.json"
    )
endif()
####################### END ##########################

add_definitions( -DCHEMISTRY_LIBRARY )
add_definitions( -DBOOST_CERTIFY_HEADER_ONLY=1 )

find_package( OpenSSL )
if ( OPENSSL_FOUND )
  add_definitions( -DOPENSSL_FOUND=1 -DPUBCHEM_REST=1 )
endif()

if ( MSVC )
  add_definitions(
    -D_SCL_SECURE_NO_WARNINGS
    )
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
  chemistry_global.hpp
  chemistryconstants.hpp
  chemconnection.cpp    chemconnection.hpp
  chemistryplugin.cpp   chemistryplugin.hpp
  chemquery.cpp         chemquery.hpp
  chemschema.cpp        chemschema.hpp
  chemspider.cpp        chemspider.hpp
  document.cpp          document.hpp
  mainwindow.cpp        mainwindow.hpp
  mode.cpp              mode.hpp
  moltablewnd.cpp       moltablewnd.hpp
  queryform.cpp         queryform.hpp
  sqledit.cpp           sqledit.hpp
  sqleditform.cpp       sqleditform.hpp
  rxneditform.cpp       rxneditform.hpp
  pubchemwnd.cpp        pubchemwnd.hpp
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
  Core
  Qt::Core
  Qt::Svg
  ${RDKit_LIBRARIES}
  ${Boost_LIBRARIES}
  pug
#  certify::sshlib
)

if ( APPLE )
  set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}"
    )
else()
  qtplatz_install_rpath( _rpath ${PROJECT_NAME} "${_plugin_install_dir}")
  set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${_plugin_output_dir}"
    INSTALL_RPATH "${_rpath}"
    )
endif()

install( TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION "${_plugin_install_dir}" COMPONENT plugins
  LIBRARY DESTINATION "${_plugin_install_dir}" COMPONENT plugins
  ARCHIVE DESTINATION lib/qtplatz COMPONENT libraries )

if ( IDE_VERSION VERSION_LESS_EQUAL "3.2.81" )
  install( FILES
    "${QTPLATZ_PLUGIN_DIRECTORY}/${PROVIDER}/${PROJECT_NAME}.pluginspec"
    DESTINATION lib/qtplatz/plugins/MS-Cheminformatics COMPONENT plugins )
endif()
